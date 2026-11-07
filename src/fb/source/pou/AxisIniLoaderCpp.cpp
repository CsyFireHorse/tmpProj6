#include "AxisIniLoader.hpp"

#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <set>
#include <string>

#include "AxisIniLoaderInternal.hpp"
#include "AxisJsonLoader.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "string.h"

class AxisIniLoader::Impl {
   public:
    class Reader : public INIReader {
       public:
        Reader(const std::string& source, bool is_content) : INIReader(source, is_content) {}

        // Expose parsed values only to the private loader implementation.
        const std::map<std::string, std::string>& values() const { return _values; }
    };

    Impl(const char* source, bool is_content)
        : source(source ? source : ""), is_content(is_content), reader(this->source, is_content) {}

    std::string source;
    bool is_content;
    Reader reader;
};

namespace {

int parse_binding_address(const std::string& text, uint32_t* address) {
    if (!address || text.empty()) return ZMC_ERR_INI_VALUE_FORMAT;
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
        if (!std::isdigit(static_cast<unsigned char>(*it))) return ZMC_ERR_INI_VALUE_FORMAT;
    }
    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = strtoul(text.c_str(), &end, 10);
    if (errno == ERANGE || !end || *end != '\0' || parsed == 0 ||
        parsed > static_cast<unsigned long>(std::numeric_limits<uint32_t>::max())) {
        return ZMC_ERR_INI_VALUE_RANGE;
    }
    *address = static_cast<uint32_t>(parsed);
    return 0;
}

int parse_io_binding_integer(const std::string& text, long* value) {
    if (!value || text.empty()) return ZMC_ERR_INI_VALUE_FORMAT;
    errno = 0;
    char* end = nullptr;
    const long parsed = strtol(text.c_str(), &end, 0);
    if (!end || end == text.c_str() || *end != '\0') return ZMC_ERR_INI_VALUE_FORMAT;
    if (errno == ERANGE) return ZMC_ERR_INI_VALUE_RANGE;
    *value = parsed;
    return 0;
}

int parse_io_binding_mask(const std::string& text, uint32_t* mask) {
    if (!mask || text.empty()) return ZMC_ERR_INI_VALUE_FORMAT;
    if (text[0] == '-') return ZMC_ERR_INI_VALUE_RANGE;

    const char* number_begin = text.c_str();
    int base = 10;
    if (text.size() >= 2u && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        if (text.size() == 2u) return ZMC_ERR_INI_VALUE_FORMAT;
        number_begin += 2;
        base = 16;
    }

    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = strtoul(number_begin, &end, base);
    if (!end || end == number_begin || *end != '\0') return ZMC_ERR_INI_VALUE_FORMAT;
    if (errno == ERANGE ||
        parsed > static_cast<unsigned long>(std::numeric_limits<uint32_t>::max())) {
        return ZMC_ERR_INI_VALUE_RANGE;
    }

    const uint32_t parsed_mask = static_cast<uint32_t>(parsed);
    if (parsed_mask == 0u || (parsed_mask & (parsed_mask - 1u)) != 0u) {
        return ZMC_ERR_IO_MASK_INVALID;
    }
    *mask = parsed_mask;
    return 0;
}

// Parse one raw DO/AO initial value as a decimal or hexadecimal uint32.
int parse_io_binding_initial_value(const std::string& text, uint32_t* value) {
    if (!value || text.empty()) return ZMC_ERR_INI_VALUE_FORMAT;
    if (text[0] == '-') return ZMC_ERR_INI_VALUE_RANGE;

    const char* number_begin = text.c_str();
    int base = 10;
    if (text.size() >= 2u && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        if (text.size() == 2u) return ZMC_ERR_INI_VALUE_FORMAT;
        number_begin += 2;
        base = 16;
    }

    if (*number_begin == '-') return ZMC_ERR_INI_VALUE_RANGE;

    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = strtoul(number_begin, &end, base);
    if (!end || end == number_begin || *end != '\0') return ZMC_ERR_INI_VALUE_FORMAT;
    if (errno == ERANGE ||
        parsed > static_cast<unsigned long>(std::numeric_limits<uint32_t>::max())) {
        return ZMC_ERR_INI_VALUE_RANGE;
    }
    *value = static_cast<uint32_t>(parsed);
    return 0;
}

bool equals_ascii_case_insensitive(const std::string& text, const char* expected) {
    if (!expected) return false;
    std::string::size_type i = 0;
    for (; i < text.size() && expected[i] != '\0'; ++i) {
        const unsigned char actual_char = static_cast<unsigned char>(text[i]);
        const unsigned char expected_char = static_cast<unsigned char>(expected[i]);
        if (std::tolower(actual_char) != std::tolower(expected_char)) return false;
    }
    return i == text.size() && expected[i] == '\0';
}

int parse_io_source_kind(const std::string& text, zmc::IoSourceKind* source_kind) {
    if (!source_kind) return ZMC_ERR_HOST_INVALID_ARG;
    // Names follow IoSourceKind values; IO_SOURCE_KIND_END is not configurable.
    static const char* const names[zmc::IO_SOURCE_KIND_END] = {
        "nop", "pdo", "virtual", "gpio", "modbus",
    };
    for (uint8_t value = 0; value < static_cast<uint8_t>(zmc::IO_SOURCE_KIND_END); ++value) {
        if (!equals_ascii_case_insensitive(text, names[value])) continue;
        *source_kind = static_cast<zmc::IoSourceKind>(value);
        return 0;
    }
    return ZMC_ERR_INI_VALUE_FORMAT;
}

std::string trim_cam_field(const std::string& text) {
    std::string::size_type begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    std::string::size_type end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(begin, end - begin);
}

int split_cam_fields(const std::string& text, int field_count, std::string* fields) {
    if (!fields || field_count <= 0) return ZMC_ERR_INI_VALUE_FORMAT;
    std::string::size_type begin = 0;
    for (int i = 0; i < field_count; ++i) {
        const std::string::size_type comma = text.find(',', begin);
        if ((i < field_count - 1 && comma == std::string::npos) ||
            (i == field_count - 1 && comma != std::string::npos)) {
            return ZMC_ERR_INI_VALUE_FORMAT;
        }
        const std::string::size_type end = comma == std::string::npos ? text.size() : comma;
        fields[i] = trim_cam_field(text.substr(begin, end - begin));
        if (fields[i].empty()) return ZMC_ERR_INI_VALUE_FORMAT;
        begin = end + 1;
    }
    return 0;
}

int parse_cam_unsigned_decimal(const std::string& text, unsigned long maximum,
                               unsigned long* value) {
    if (!value || text.empty()) return ZMC_ERR_INI_VALUE_FORMAT;
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
        if (!std::isdigit(static_cast<unsigned char>(*it))) return ZMC_ERR_INI_VALUE_FORMAT;
    }

    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = strtoul(text.c_str(), &end, 10);
    if (errno == ERANGE || !end || *end != '\0' || parsed > maximum) {
        return ZMC_ERR_INI_VALUE_RANGE;
    }
    *value = parsed;
    return 0;
}

int parse_cam_version(const std::string& text, int* major, int* minor, int* patch) {
    if (!major || !minor || !patch) return ZMC_ERR_INI_VALUE_FORMAT;
    const std::string::size_type first_dot = text.find('.');
    if (first_dot == std::string::npos) return ZMC_ERR_INI_VALUE_FORMAT;
    const std::string::size_type second_dot = text.find('.', first_dot + 1);
    if (second_dot == std::string::npos || text.find('.', second_dot + 1) != std::string::npos) {
        return ZMC_ERR_INI_VALUE_FORMAT;
    }

    const std::string fields[] = {
        text.substr(0, first_dot),
        text.substr(first_dot + 1, second_dot - first_dot - 1),
        text.substr(second_dot + 1),
    };
    unsigned long values[3] = {0};
    for (int i = 0; i < 3; ++i) {
        const int ret = parse_cam_unsigned_decimal(
            fields[i], static_cast<unsigned long>(std::numeric_limits<int>::max()), &values[i]);
        if (ret != 0) return ret;
    }
    *major = static_cast<int>(values[0]);
    *minor = static_cast<int>(values[1]);
    *patch = static_cast<int>(values[2]);
    return 0;
}

int parse_cam_finite_double(const std::string& text, double* value) {
    if (!value || text.empty()) return ZMC_ERR_INI_VALUE_FORMAT;
    errno = 0;
    char* end = nullptr;
    const double parsed = strtod(text.c_str(), &end);
    if (!end || end == text.c_str() || *end != '\0') return ZMC_ERR_INI_VALUE_FORMAT;
    if (errno == ERANGE || !std::isfinite(parsed)) return ZMC_ERR_INI_VALUE_RANGE;
    *value = parsed;
    return 0;
}

int parse_cam_xyva_point(const std::string& text, CamXyva* point) {
    if (!point) return ZMC_ERR_INI_VALUE_FORMAT;
    std::string fields[4];
    int ret = split_cam_fields(text, 4, fields);
    if (ret != 0) return ret;

    CamXyva parsed = CamXyva();
    ret = parse_cam_finite_double(fields[0], &parsed.dX);
    if (ret != 0) return ret;
    ret = parse_cam_finite_double(fields[1], &parsed.dY);
    if (ret != 0) return ret;
    ret = parse_cam_finite_double(fields[2], &parsed.dV);
    if (ret != 0) return ret;
    ret = parse_cam_finite_double(fields[3], &parsed.dA);
    if (ret != 0) return ret;
    *point = parsed;
    return 0;
}

int parse_cam_tappet(const std::string& text, Tappet* tappet) {
    if (!tappet) return ZMC_ERR_INI_VALUE_FORMAT;
    std::string fields[6];
    int ret = split_cam_fields(text, 6, fields);
    if (ret != 0) return ret;

    unsigned long group_id = 0;
    unsigned long tappet_type = 0;
    unsigned long tappet_action = 0;
    unsigned long delay = 0;
    unsigned long duration = 0;
    double position = 0.0;
    ret = parse_cam_unsigned_decimal(fields[0], std::numeric_limits<uint16_t>::max(), &group_id);
    if (ret != 0) return ret;
    ret = parse_cam_unsigned_decimal(fields[1], TAPPET_neg, &tappet_type);
    if (ret != 0) return ret;
    ret = parse_cam_unsigned_decimal(fields[2], TAPPETACTION_time, &tappet_action);
    if (ret != 0) return ret;
    ret = parse_cam_finite_double(fields[3], &position);
    if (ret != 0) return ret;
    ret = parse_cam_unsigned_decimal(fields[4], std::numeric_limits<uint32_t>::max(), &delay);
    if (ret != 0) return ret;
    ret = parse_cam_unsigned_decimal(fields[5], std::numeric_limits<uint32_t>::max(), &duration);
    if (ret != 0) return ret;

    Tappet parsed = Tappet();
    parsed.iGroupID = static_cast<uint16_t>(group_id);
    parsed.ctt = static_cast<uint16_t>(tappet_type);
    parsed.cta = static_cast<uint16_t>(tappet_action);
    parsed.x = position;
    parsed.dwDelay = static_cast<uint32_t>(delay);
    parsed.dwDuration = static_cast<uint32_t>(duration);
    parsed.dwActive = 0xffffffffu;
    *tappet = parsed;
    return 0;
}

int parse_cam_record_index(const std::string& key, const char* prefix, int maximum, int* index) {
    if (!prefix || !index || maximum <= 0) return ZMC_ERR_INI_KEY;
    const std::string prefix_text(prefix);
    if (key.compare(0, prefix_text.size(), prefix_text) != 0) return ZMC_ERR_INI_KEY;
    const std::string index_text = key.substr(prefix_text.size());
    if (index_text.empty()) return ZMC_ERR_INI_KEY;

    unsigned long parsed_index = 0;
    const int ret = parse_cam_unsigned_decimal(
        index_text, static_cast<unsigned long>(maximum - 1), &parsed_index);
    if (ret != 0) return ZMC_ERR_INI_KEY;
    *index = static_cast<int>(parsed_index);
    if (key != prefix_text + std::to_string(*index)) return ZMC_ERR_INI_KEY;
    return 0;
}

}  // namespace

AxisIniLoader::AxisIniLoader(const char* source, bool is_content)
    : impl_(new Impl(source, is_content)) {}

AxisIniLoader::~AxisIniLoader() {}

int AxisIniLoader::load_binding(AxisBindingConfig* cfg) const {
    if (!cfg) return ZMC_ERR_HOST_INVALID_ARG;
    const INIReader& reader = impl_->reader;
    if (reader.ParseError() != 0) return ZMC_ERR_INI_PARSE;

    AxisBindingConfig parsed = AxisBindingConfig();
    if (!axis_ini_has_section(reader, "axis_binding")) {
        *cfg = parsed;
        return 0;
    }
    parsed.section_present = 1;

    struct BindingFields {
        bool has_name;
        bool has_index;
        std::string name;
        std::string index;

        BindingFields() : has_name(false), has_index(false) {}
    };

    const std::string section_prefix = "axis_binding=";
    std::map<int, BindingFields> records;
    const std::map<std::string, std::string>& values = impl_->reader.values();
    for (std::map<std::string, std::string>::const_iterator it = values.begin();
         it != values.end(); ++it) {
        if (it->first.compare(0, section_prefix.size(), section_prefix) != 0) continue;

        const std::string key = it->first.substr(section_prefix.size());
        if (key.compare(0, 5, "axis_") != 0) continue;

        const std::string::size_type dot = key.find('.', 5);
        if (dot == std::string::npos || dot == 5) return ZMC_ERR_INI_KEY;
        const std::string axis_text = key.substr(5, dot - 5);
        for (std::string::const_iterator ch = axis_text.begin(); ch != axis_text.end(); ++ch) {
            if (!std::isdigit(static_cast<unsigned char>(*ch))) return ZMC_ERR_INI_KEY;
        }

        errno = 0;
        char* end = nullptr;
        const unsigned long parsed_axis = strtoul(axis_text.c_str(), &end, 10);
        if (errno == ERANGE || !end || *end != '\0' || parsed_axis >= AXIS_BINDING_MAX_COUNT) {
            return ZMC_ERR_INI_VALUE_RANGE;
        }

        BindingFields& fields = records[static_cast<int>(parsed_axis)];
        const std::string field = key.substr(dot + 1);
        if (field == "name") {
            fields.has_name = true;
            fields.name = it->second;
        } else if (field == "index") {
            fields.has_index = true;
            fields.index = it->second;
        } else {
            return ZMC_ERR_INI_KEY;
        }
    }
    if (records.empty()) return ZMC_ERR_INI_AXIS_BINDING_CONFIG;

    std::set<std::string> names;
    std::set<uint32_t> addresses;
    // Optional names are validated only when supplied. AI modification signature: GPT-5 Codex
    for (std::map<int, BindingFields>::const_iterator it = records.begin(); it != records.end();
         ++it) {
        const BindingFields& fields = it->second;
        if (!fields.has_index) return ZMC_ERR_INI_MISSING_KEY;
        if (fields.has_name && (fields.name.empty() || fields.name.find('\n') != std::string::npos)) {
            return ZMC_ERR_INI_VALUE_FORMAT;
        }
        if (fields.has_name && fields.name.size() >= MAX_AXIS_NAME_LEN) {
            return ZMC_ERR_INI_VALUE_RANGE;
        }
        uint32_t address = 0;
        const int address_ret = parse_binding_address(fields.index, &address);
        if (address_ret != 0) return address_ret;
        if ((fields.has_name && !names.insert(fields.name).second) ||
            !addresses.insert(address).second) {
            return ZMC_ERR_INI_DUPLICATE;
        }

        AxisBindingEntry& entry = parsed.entries[parsed.binding_count++];
        memcpy(entry.name, fields.name.c_str(), fields.name.size() + 1);
        entry.axis_index = it->first;
        entry.address = address;
    }
    *cfg = parsed;
    return 0;
}

int AxisIniLoader::load_io_binding(IoBindingConfig* cfg) const {
    if (!cfg) return ZMC_ERR_HOST_INVALID_ARG;
    const INIReader& reader = impl_->reader;
    if (reader.ParseError() != 0) return ZMC_ERR_INI_PARSE;

    IoBindingConfig parsed = IoBindingConfig();
    if (!axis_ini_has_section(reader, "io_binding")) {
        *cfg = parsed;
        return 0;
    }
    parsed.section_present = 1;

    const std::string section_prefix = "io_binding=";
    // Fixed seen tables match the fixed binding storage and avoid parser-side containers.
    uint8_t di_seen[IO_BINDING_ALIAS_CAPACITY] = {0};
    uint8_t do_seen[IO_BINDING_ALIAS_CAPACITY] = {0};
    uint8_t ai_seen[IO_BINDING_ALIAS_CAPACITY] = {0};
    uint8_t ao_seen[IO_BINDING_ALIAS_CAPACITY] = {0};
    const std::map<std::string, std::string>& values = impl_->reader.values();
    for (std::map<std::string, std::string>::const_iterator it = values.begin();
         it != values.end(); ++it) {
        if (it->first.compare(0, section_prefix.size(), section_prefix) != 0) continue;

        const std::string key = it->first.substr(section_prefix.size());
        uint8_t* seen = nullptr;
        int* binding_count = nullptr;
        IoBindingEntry* bindings = nullptr;
        std::string::size_type record_begin = 0;
        bool is_digital = false;
        bool is_output = false;
        if (key.compare(0, 3, "di_") == 0) {
            seen = di_seen;
            binding_count = &parsed.di_count;
            bindings = parsed.di_entries;
            record_begin = 3;
            is_digital = true;
        } else if (key.compare(0, 3, "do_") == 0) {
            seen = do_seen;
            binding_count = &parsed.do_count;
            bindings = parsed.do_entries;
            record_begin = 3;
            is_digital = true;
            is_output = true;
        } else if (key.compare(0, 3, "ai_") == 0) {
            seen = ai_seen;
            binding_count = &parsed.ai_count;
            bindings = parsed.ai_entries;
            record_begin = 3;
        } else if (key.compare(0, 3, "ao_") == 0) {
            seen = ao_seen;
            binding_count = &parsed.ao_count;
            bindings = parsed.ao_entries;
            record_begin = 3;
            is_output = true;
        } else {
            return ZMC_ERR_INI_KEY;
        }

        const std::string::size_type dot = key.find('.', record_begin);
        if (dot == std::string::npos || dot == record_begin) return ZMC_ERR_INI_KEY;

        const std::string field = key.substr(dot + 1);
        if (field != "source" && field != "slave_idx" && field != "index" &&
            field != "subindex" && field != "mask" &&
            !(field == "initial_value" && is_output)) {
            return ZMC_ERR_INI_KEY;
        }
        const std::string record = key.substr(0, dot);
        const std::string io_number_text = key.substr(record_begin, dot - record_begin);
        for (std::string::const_iterator ch = io_number_text.begin(); ch != io_number_text.end();
             ++ch) {
            if (!std::isdigit(static_cast<unsigned char>(*ch))) return ZMC_ERR_INI_KEY;
        }
        errno = 0;
        char* end = nullptr;
        const unsigned long io_number = strtoul(io_number_text.c_str(), &end, 10);
        if (errno == ERANGE || !end || *end != '\0') return ZMC_ERR_INI_VALUE_RANGE;
        if (io_number >= IO_BINDING_ALIAS_CAPACITY) return ZMC_ERR_IO_ALIAS_CAPACITY;

        const bool has_source = reader.HasKey("io_binding", record + ".source");
        if (!has_source) return ZMC_ERR_INI_MISSING_KEY;
        zmc::IoSourceKind source_kind = zmc::IO_SOURCE_KIND_NOP;
        const std::string source = reader.Get("io_binding", record + ".source", "");
        const int source_ret = parse_io_source_kind(source, &source_kind);
        if (source_ret != 0) return source_ret;

        const bool has_slave_idx = reader.HasKey("io_binding", record + ".slave_idx");
        const bool has_index = reader.HasKey("io_binding", record + ".index");
        const bool has_subindex = reader.HasKey("io_binding", record + ".subindex");
        const bool has_mask = reader.HasKey("io_binding", record + ".mask");
        const bool has_initial_value = reader.HasKey("io_binding", record + ".initial_value");
        if (source_kind != zmc::IO_SOURCE_KIND_PDO) {
            if (has_slave_idx || has_index || has_subindex || has_mask) {
                return ZMC_ERR_INI_IO_BINDING_CONFIG;
            }
            // The source field is the sole anchor for one non-PDO record.
            if (field != "source") continue;
        } else {
            if (!has_slave_idx || !has_index || !has_subindex) {
                return ZMC_ERR_INI_MISSING_KEY;
            }
            if (!is_digital && has_mask) return ZMC_ERR_INI_KEY;
            // The explicit source field is the sole anchor for one PDO record.
            if (field != "source") continue;
        }

        if (seen[io_number]) return ZMC_ERR_INI_DUPLICATE;
        seen[io_number] = 1u;

        IoBindingEntry& entry = bindings[(*binding_count)++];
        entry.io_number = static_cast<uint32_t>(io_number);
        entry.source_kind = source_kind;
        entry.initial_value = 0u;
        entry.has_initial_value = false;
        if (has_initial_value) {
            const int initial_value_ret = parse_io_binding_initial_value(
                reader.Get("io_binding", record + ".initial_value", ""), &entry.initial_value);
            if (initial_value_ret != 0) return initial_value_ret;
            if (key.compare(0, 3, "do_") == 0 && entry.initial_value > 1u) {
                return ZMC_ERR_INI_VALUE_RANGE;
            }
            entry.has_initial_value = true;
        }
        if (source_kind != zmc::IO_SOURCE_KIND_PDO) continue;

        long slave_idx = 0;
        long index = 0;
        long subindex = 0;
        int value_ret = parse_io_binding_integer(
            reader.Get("io_binding", record + ".slave_idx", ""), &slave_idx);
        if (value_ret != 0) return value_ret;
        value_ret = parse_io_binding_integer(reader.Get("io_binding", record + ".index", ""),
                                             &index);
        if (value_ret != 0) return value_ret;
        value_ret = parse_io_binding_integer(
            reader.Get("io_binding", record + ".subindex", ""), &subindex);
        if (value_ret != 0) return value_ret;
        if (slave_idx < 0 || slave_idx > std::numeric_limits<uint16_t>::max() || index < 0 ||
            index > std::numeric_limits<uint16_t>::max() || subindex < 0 ||
            subindex > std::numeric_limits<uint8_t>::max()) {
            return ZMC_ERR_INI_VALUE_RANGE;
        }

        entry.key.slave_idx = static_cast<uint16_t>(slave_idx);
        entry.key.index = static_cast<uint16_t>(index);
        entry.key.subindex = static_cast<uint8_t>(subindex);
        if (has_mask) {
            value_ret = parse_io_binding_mask(
                reader.Get("io_binding", record + ".mask", ""), &entry.mask);
            if (value_ret != 0) return value_ret;
        }
    }

    if (parsed.di_count == 0 && parsed.do_count == 0 && parsed.ai_count == 0 &&
        parsed.ao_count == 0) {
        return ZMC_ERR_INI_IO_BINDING_CONFIG;
    }

    *cfg = parsed;
    return 0;
}

int AxisIniLoader::load_cam_table(IniCamTable* cam) const {
    if (!cam || cam->table_index < 0) return ZMC_ERR_HOST_INVALID_ARG;
    const INIReader& reader = impl_->reader;
    if (reader.ParseError() != 0) return ZMC_ERR_INI_PARSE;

    const std::string section = "cam_table_" + std::to_string(cam->table_index);
    if (!axis_ini_has_section(reader, section)) return ZMC_ERR_INI_MISSING_SECTION;

    static const char* const required_keys[] = {
        "version", "xStart", "xEnd", "xyva_count", "tappet_count",
    };
    for (size_t i = 0; i < sizeof(required_keys) / sizeof(required_keys[0]); ++i) {
        if (!reader.HasKey(section, required_keys[i])) return ZMC_ERR_INI_MISSING_KEY;
        if (reader.Get(section, required_keys[i], "").find('\n') != std::string::npos) {
            return ZMC_ERR_INI_DUPLICATE;
        }
    }

    int ver_major = 0;
    int ver_minor = 0;
    int ver_patch = 0;
    unsigned long xyva_count = 0;
    unsigned long tappet_count = 0;
    double x_start = 0.0;
    double x_end = 0.0;
    int ret = parse_cam_version(reader.Get(section, "version", ""), &ver_major, &ver_minor,
                                &ver_patch);
    if (ret != 0) return ret;
    ret = parse_cam_finite_double(reader.Get(section, "xStart", ""), &x_start);
    if (ret != 0) return ret;
    ret = parse_cam_finite_double(reader.Get(section, "xEnd", ""), &x_end);
    if (ret != 0) return ret;
    ret = parse_cam_unsigned_decimal(reader.Get(section, "xyva_count", ""),
                                     INI_CAM_XYVA_MAX_COUNT, &xyva_count);
    if (ret != 0) return ret;
    if (xyva_count == 0) return ZMC_ERR_INI_VALUE_RANGE;
    ret = parse_cam_unsigned_decimal(reader.Get(section, "tappet_count", ""),
                                     INI_CAM_TAPPET_MAX_COUNT, &tappet_count);
    if (ret != 0) return ret;

    uint8_t xyva_seen[INI_CAM_XYVA_MAX_COUNT] = {0};
    uint8_t tappet_seen[INI_CAM_TAPPET_MAX_COUNT] = {0};
    const std::string section_prefix = section + "=";
    const std::map<std::string, std::string>& values = impl_->reader.values();
    for (std::map<std::string, std::string>::const_iterator it = values.begin();
         it != values.end(); ++it) {
        if (it->first.compare(0, section_prefix.size(), section_prefix) != 0) continue;

        const std::string key = it->first.substr(section_prefix.size());
        if (key == "version" || key == "xstart" || key == "xend" || key == "xyva_count" ||
            key == "tappet_count") {
            continue;
        }

        if (key.compare(0, 5, "xyva_") == 0) {
            int index = 0;
            ret = parse_cam_record_index(key, "xyva_", INI_CAM_XYVA_MAX_COUNT, &index);
            if (ret != 0) return ret;
            if (index >= static_cast<int>(xyva_count)) return ZMC_ERR_INI_VALUE_RANGE;
            if (xyva_seen[index] || it->second.find('\n') != std::string::npos) {
                return ZMC_ERR_INI_DUPLICATE;
            }
            CamXyva parsed_point = CamXyva();
            ret = parse_cam_xyva_point(it->second, &parsed_point);
            if (ret != 0) return ret;
            xyva_seen[index] = 1u;
            continue;
        }

        if (key.compare(0, 7, "tappet_") == 0) {
            int index = 0;
            ret = parse_cam_record_index(key, "tappet_", INI_CAM_TAPPET_MAX_COUNT, &index);
            if (ret != 0) return ret;
            if (index >= static_cast<int>(tappet_count)) return ZMC_ERR_INI_VALUE_RANGE;
            if (tappet_seen[index] || it->second.find('\n') != std::string::npos) {
                return ZMC_ERR_INI_DUPLICATE;
            }
            Tappet parsed_tappet = Tappet();
            ret = parse_cam_tappet(it->second, &parsed_tappet);
            if (ret != 0) return ret;
            tappet_seen[index] = 1u;
            continue;
        }

        return ZMC_ERR_INI_KEY;
    }

    for (int i = 0; i < static_cast<int>(xyva_count); ++i) {
        if (!xyva_seen[i]) return ZMC_ERR_INI_MISSING_KEY;
    }
    for (int i = 0; i < static_cast<int>(tappet_count); ++i) {
        if (!tappet_seen[i]) return ZMC_ERR_INI_MISSING_KEY;
    }

    // Reparse only after validating the complete section so failures leave cam unchanged.
    // AI modification signature: GPT-5 Codex
    for (int i = 0; i < static_cast<int>(xyva_count); ++i) {
        const std::string key = "xyva_" + std::to_string(i);
        ret = parse_cam_xyva_point(reader.Get(section, key, ""), &cam->xyva[i]);
        if (ret != 0) return ret;
    }
    for (int i = 0; i < static_cast<int>(tappet_count); ++i) {
        const std::string key = "tappet_" + std::to_string(i);
        ret = parse_cam_tappet(reader.Get(section, key, ""), &cam->tappets[i]);
        if (ret != 0) return ret;
    }

    static_assert(sizeof(SMC_CAMTappet) == sizeof(Tappet),
                  "Tappet layout must match MC_CAM_TABLE");
    cam->init();
    cam->ver_major = ver_major;
    cam->ver_minor = ver_minor;
    cam->ver_patch = ver_patch;
    cam->xStart = x_start;
    cam->xEnd = x_end;
    cam->nElements = static_cast<int>(xyva_count);
    cam->pce = reinterpret_cast<ZMC_BYTE*>(cam->xyva);
    cam->nTappets = static_cast<int>(tappet_count);
    cam->pt = tappet_count > 0 ? reinterpret_cast<SMC_CAMTappet*>(cam->tappets) : nullptr;
    return 0;
}

int AxisIniLoader::load_base(AxisCommonConfig* cfg) const {
    return axis_ini_load_base_from_reader(impl_->reader, impl_->source.c_str(), impl_->is_content,
                                          cfg);
}

int AxisIniLoader::load_axis(AxisCiaConfig* cfg, const char* section) const {
    if (!cfg || !section) return ZMC_ERR_HOST_INVALID_ARG;
    return axis_ini_load_axis_from_reader(impl_->reader, impl_->source.c_str(), *cfg, section);
}

int AxisIniLoader::load_axis_by_index(int index, const AxisCommonConfig* common_cfg,
                                      AxisCiaConfig* cfg) const {
    if (!common_cfg || !cfg || index < 0) return ZMC_ERR_HOST_INVALID_ARG;
    const AxisCommonConfig& ini_cfg = *common_cfg;
    const std::string key = "axis_" + std::to_string(index);
    if (index < static_cast<int>(ini_cfg.axis_file_num) && ini_cfg.axis_file_array[index]) {
        const std::string json_file =
            std::string(ini_cfg.axis_file_dir) + ini_cfg.axis_file_array[index];
        printf("%d load %s\n", index, json_file.c_str());
        const int ret = zmc::load_axis_json(json_file.c_str(), *cfg);
        if (ret != 0) return ret;
    }
    return load_axis(cfg, key.c_str());
}
