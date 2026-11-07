#include "utils/eni_motor_map.hpp"

#include <algorithm>
#include <limits>

#include "common/zmc/zmc_predef.h"
// #include "common/bar_space_mc_cmd.h"
#include <thirdlib/nlohmann/json.hpp>
namespace eni {
namespace detail {

// Extract JSON object substring after @@##ENI_INFO##@@ marker (implementation detail; not in
// headers).
MCCommResult ExtractEniListJson(const std::string& xml_blob, std::string* json_text) {
    if (json_text == nullptr) {
        return ZMC_ERR_DEV_NULL_ARG;
    }

    const std::string marker = "@@##ENI_INFO##@@";
    const size_t marker_pos = xml_blob.find(marker);
    if (marker_pos == std::string::npos) {
        return ZMC_ERR_ENI_JSON_NOT_FOUND;
    }

    const size_t json_start = xml_blob.find('{', marker_pos + marker.size());
    if (json_start == std::string::npos) {
        return ZMC_ERR_ENI_JSON_INVALID;
    }

    bool in_string = false;
    bool escaped = false;
    int depth = 0;
    size_t json_end = std::string::npos;
    // Find the matching closing brace without being fooled by braces inside JSON strings.
    for (size_t i = json_start; i < xml_blob.size(); ++i) {
        const char ch = xml_blob[i];
        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (ch == '\\') {
                escaped = true;
            } else if (ch == '"') {
                in_string = false;
            }
            continue;
        }

        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch == '{') {
            ++depth;
            continue;
        }
        if (ch == '}') {
            --depth;
            if (depth == 0) {
                json_end = i;
                break;
            }
        }
    }

    if (json_end == std::string::npos) {
        return ZMC_ERR_ENI_JSON_INVALID;
    }

    *json_text = xml_blob.substr(json_start, json_end - json_start + 1);
    return MC_COMM_S_OK;
}

uint32_t ParseU32(const nlohmann::json& v, uint32_t fallback = 0) {
    // ENI_INFO can carry numeric fields as JSON numbers or strings from generator tools.
    if (v.is_number_unsigned()) {
        return v.get<uint32_t>();
    }
    if (v.is_number_integer()) {
        const int64_t x = v.get<int64_t>();
        return x < 0 ? fallback : static_cast<uint32_t>(x);
    }
    if (v.is_string()) {
        const std::string s = v.get<std::string>();
        try {
            return static_cast<uint32_t>(std::stoul(s, nullptr, 0));
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

std::string ParseString(const nlohmann::json& v) {
    if (v.is_string()) {
        return v.get<std::string>();
    }
    if (v.is_number_integer()) {
        return std::to_string(v.get<int64_t>());
    }
    if (v.is_number_unsigned()) {
        return std::to_string(v.get<uint64_t>());
    }
    return std::string();
}

int32_t BitOffsetToByte(int64_t bit_offset) {
    // The downstream PDO adapters work on byte offsets only.
    if (bit_offset < 0 || (bit_offset % 8) != 0) {
        return -1;
    }
    const int64_t byte_offset = bit_offset / 8;
    if (byte_offset > std::numeric_limits<int32_t>::max()) {
        return -1;
    }
    return static_cast<int32_t>(byte_offset);
}

void InitOffset(pi_offset_config* cfg) {
    cfg->slave_idx = 0;
    cfg->slave_channel_idx = 0;
    cfg->po_mode_offset = -1;
    cfg->po_ctrl_word_offset = -1;
    cfg->po_p_offset = -1;
    cfg->po_v_offset = -1;
    cfg->po_t_offset = -1;
    cfg->pi_mode_offset = -1;
    cfg->pi_stat_offset = -1;
    cfg->pi_error_offset = -1;
    cfg->pi_p_offset = -1;
    cfg->pi_v_offset = -1;
    cfg->pi_t_offset = -1;
}

void ApplyEntryToOffsets(const PdoEntry& e, bool is_output, pi_offset_config* cfg) {
    const int32_t byte_ofs = BitOffsetToByte(static_cast<int64_t>(e.bit_offset));
    if (byte_ofs < 0) {
        return;
    }
    // Translate CiA402 PDO object indices into the offset slots consumed by AxisFbBridge.
    if (is_output) {
        switch (e.index) {
            case 0x6040:
                cfg->po_ctrl_word_offset = byte_ofs;
                break;
            case 0x607A:
                cfg->po_p_offset = byte_ofs;
                break;
            case 0x60FF:
                cfg->po_v_offset = byte_ofs;
                break;
            case 0x6071:
                cfg->po_t_offset = byte_ofs;
                break;
            case 0x6060:
                cfg->po_mode_offset = byte_ofs;
                break;
            default:
                break;
        }
    } else {
        switch (e.index) {
            case 0x6041:
                cfg->pi_stat_offset = byte_ofs;
                break;
            case 0x6064:
                cfg->pi_p_offset = byte_ofs;
                break;
            case 0x606C:
                cfg->pi_v_offset = byte_ofs;
                break;
            case 0x6077:
                cfg->pi_t_offset = byte_ofs;
                break;
            case 0x6061:
                cfg->pi_mode_offset = byte_ofs;
                break;
            case 0x603F:
                cfg->pi_error_offset = byte_ofs;
                break;
            default:
                break;
        }
    }
}

bool HasIndex(const std::vector<PdoEntry>& entries, uint32_t idx) {
    return std::any_of(entries.begin(), entries.end(),
                       [idx](const PdoEntry& e) { return e.index == idx; });
}

PdoEntry ParsePdoEntry(const nlohmann::json& item) {
    PdoEntry e;
    e.channel = ParseString(item.value("channel", nlohmann::json()));
    e.name = ParseString(item.value("name", nlohmann::json()));
    e.index = ParseU32(item.value("index", nlohmann::json()), 0);
    e.subindex = static_cast<uint16_t>(ParseU32(item.value("subindex", nlohmann::json()), 0));
    e.type = ParseString(item.value("type", nlohmann::json()));
    e.bit_size = ParseU32(item.value("bit_size", nlohmann::json()), 0);
    e.bit_offset = ParseU32(item.value("bit_offset", nlohmann::json()), 0);
    return e;
}

// Fills *out from parsed JSON root. Caller ensures root is object.
MCCommResult BuildDescriptorFromRoot(const nlohmann::json& root, EniDescriptor* out) {
    if (out == nullptr) {
        return ZMC_ERR_DEV_NULL_ARG;
    }
    out->master = MasterInfo{};
    out->slaves.clear();

    if (root.contains("master") && root["master"].is_object()) {
        const auto& master = root["master"];
        if (master.contains("base") && master["base"].is_object()) {
            out->master.cycle_time_us =
                ParseU32(master["base"].value("cycle_time", nlohmann::json()), 1000);
        }
    }

    if (!root.contains("slaves") || !root["slaves"].is_array()) {
        return ZMC_ERR_ENI_SLAVE_INVALID;
    }
    const auto& slaves = root["slaves"];
    if (slaves.empty()) {
        return ZMC_ERR_ENI_SLAVE_INVALID;
    }

    int slave_idx = 0;
    for (const auto& s : slaves) {
        if (!s.is_object()) {
            continue;
        }
        SlaveInfo si;
        si.index = slave_idx++;
        si.name = ParseString(s.value("name", nlohmann::json()));
        si.vendor_id = ParseU32(s.value("vendor_id", nlohmann::json()), 0);
        si.vendor_name = ParseString(s.value("vendor_name", nlohmann::json()));
        si.type = ParseString(s.value("type", nlohmann::json()));
        si.product_code = ParseU32(s.value("product_code", nlohmann::json()), 0);
        si.revision_no = ParseU32(s.value("revision_no", nlohmann::json()), 0);
        si.product_revision = ParseString(s.value("product_revision", nlohmann::json()));
        si.phys_addr = static_cast<uint16_t>(ParseU32(s.value("phys_addr", nlohmann::json()), 0));
        si.auto_inc_addr =
            static_cast<uint16_t>(ParseU32(s.value("auto_inc_addr", nlohmann::json()), 0));
        InitOffset(&si.offsets);
        si.offsets.slave_idx = static_cast<uint8_t>(si.index);

        const auto parse_entries = [&](const char* key, std::vector<PdoEntry>* dst, bool output) {
            if (!s.contains(key) || !s[key].is_array()) {
                return;
            }
            // Keep every parsed PDO entry for descriptor export, while also filling known motion
            // offsets.
            for (const auto& p : s[key]) {
                if (!p.is_object()) {
                    continue;
                }
                PdoEntry e = ParsePdoEntry(p);
                dst->push_back(e);
                ApplyEntryToOffsets(e, output, &si.offsets);
            }
        };
        parse_entries("outputs", &si.outputs, true);
        parse_entries("inputs", &si.inputs, false);

        si.is_servo = HasIndex(si.outputs, 0x6040) && HasIndex(si.inputs, 0x6041);
        out->slaves.push_back(si);
    }

    if (out->slaves.empty()) {
        return ZMC_ERR_ENI_SLAVE_INVALID;
    }

    size_t servo_axes = 0;
    for (const auto& sl : out->slaves) {
        if (sl.is_servo) {
            ++servo_axes;
        }
    }
    // Axis limit is enforced on detected servo slaves, not on all EtherCAT slaves.
    if (servo_axes > MAX_NUM_SERVOS) {
        return ZMC_ERR_ENI_AXIS_OVERFLOW;
    }

    return MC_COMM_S_OK;
}

MCCommResult DescriptorFromJsonText(const std::string& json_text, EniDescriptor* out) {
    try {
        nlohmann::json root = nlohmann::json::parse(json_text, nullptr, false);
        if (root.is_discarded() || !root.is_object()) {
            return ZMC_ERR_ENI_JSON_INVALID;
        }
        return BuildDescriptorFromRoot(root, out);
    } catch (const std::exception&) {
        return ZMC_ERR_ENI_JSON_INVALID;
    }
}

MCCommResult DescriptorFromXmlBlob(const std::string& xml_blob, EniDescriptor* out) {
    if (out == nullptr) {
        return ZMC_ERR_DEV_NULL_ARG;
    }

    std::string json_text;
    const MCCommResult ex = ExtractEniListJson(xml_blob, &json_text);
    if (ex != MC_COMM_S_OK) {
        return ex;
    }
    return DescriptorFromJsonText(json_text, out);
}

}  // namespace detail

MCCommResult LoadDescriptorFromEniXmlBlob(const std::string& xml_blob, EniDescriptor* out) {
    return detail::DescriptorFromXmlBlob(xml_blob, out);
}

}  // namespace eni
