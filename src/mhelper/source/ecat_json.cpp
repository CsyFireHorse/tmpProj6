#include "ecat_json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>

#include "common/thirdlib/nlohmann/json.hpp"
#include "eni_json_slice.h"
#include "fbt_errorcode_def.h"

// #include <assert.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <fcntl.h>
#define LEN(x) sizeof(x) / sizeof(*(x))

using namespace std;
using json = nlohmann::json;

#define ENI_MAX_SIZE (1024 * 1024u)
#define SLAVES_CONFIG_DEFAULT "slaves_cfg_default.json"
#define ECAT_VALUE_ADD(json, key) ((json.contains(key)) ? (json.at(key)) : (" "))

namespace zmc {
static string json_text_or_number(const json& object, const char* key) {
    if (!object.contains(key)) return " ";
    const json& value = object.at(key);
    return value.is_number() ? value.dump() : value.get<string>();
}

// Slave order does not define PDO process-image order.
uint16_t SlaveInfo::get_rx_end_addr() const {
    uint16_t end_addr = 0;
    for (uint32_t slave_idx = 0; slave_idx < slaves.size(); ++slave_idx) {
        const vector<pdo_info_t> &entries = slaves[slave_idx].outputs;
        for (uint32_t pdo_idx = 0; pdo_idx < entries.size(); ++pdo_idx) {
            const uint16_t pdo_end_addr = entries[pdo_idx].get_end_addr();
            if (pdo_end_addr > end_addr) {
                end_addr = pdo_end_addr;
            }
        }
    }
    return end_addr;
}

uint16_t SlaveInfo::get_tx_end_addr() const {
    uint16_t end_addr = 0;
    for (uint32_t slave_idx = 0; slave_idx < slaves.size(); ++slave_idx) {
        const vector<pdo_info_t> &entries = slaves[slave_idx].inputs;
        for (uint32_t pdo_idx = 0; pdo_idx < entries.size(); ++pdo_idx) {
            const uint16_t pdo_end_addr = entries[pdo_idx].get_end_addr();
            if (pdo_end_addr > end_addr) {
                end_addr = pdo_end_addr;
            }
        }
    }
    return end_addr;
}

class ecat_json {
   public:
    ecat_json();
    ecat_json(const char *xml_file);
    ~ecat_json() {};
    int load(const char *xml_file);
    int load_jsonstr(const char *ptr);

   private:
    int pdo_obj_push(vector<pdo_info_t> &pdo_obj, const json &json_pdo, bool is_output,
                     uint8_t vers = 0);
    int _pdo_obj_push_ver0(vector<pdo_info_t> &pdo_obj, const json &json_pdo, bool is_output);
    int _pdo_obj_push_ver2(vector<pdo_info_t> &pdo_obj, const json &json_pdo, bool is_output);
    int slave_info_push(SLAVES_INFO_t &slave, const json &json_slaves, uint8_t vers = 0);

   private:
    uint8_t xml_file_select;
    uint32_t outputs_offset;
    uint32_t inputs_offset;

   public:
    MasterInfo master;
    SlaveInfo slvInfo;
};

int load_slaves_from_eni_xml_json(const char *xml_file, SlaveInfo &slaves, MasterInfo *mInfo) {
    ecat_json eni_json;
    int errCode = eni_json.load(xml_file);
    if (errCode) return errCode;
    int sz = eni_json.slvInfo.size();
    if (sz <= 0) {
        printf("Eni or slaves num %d error\n", sz);
        return ZMC_ERR_ENI_SLAVE_INVALID;
    }
    slaves = std::move(eni_json.slvInfo);
    if (mInfo) {
        *mInfo = eni_json.master;
    }

    return 0;
}
inline int32_t decHexStr2num(const char *str) {
    if (strncmp(str, "0x", 2) == 0) {
        // char* p = "0x1a";
        int nValude = 0;
        int ret = sscanf(str, "%x", &nValude);
        return nValude;
    } else {
        return atoi(str);
    }
}

inline int32_t hexstr2num(const std::string &str) {
    if (str.size() <= 2) return 0;
    int nValude = 0;
    int ret = sscanf(str.c_str(), "%x", &nValude);
    return nValude;
}

ecat_json::ecat_json() : xml_file_select(0), outputs_offset(0), inputs_offset(0) {}
ecat_json::ecat_json(const char *xml_file)
    : xml_file_select(0), outputs_offset(0), inputs_offset(0) {
    load(xml_file);
}
int ecat_json::load(const char *xml_file) {
    std::string json_text;
    const int ret = extract_eni_json_text(xml_file, json_text);
    if (ret) {
        return ret;
    }
    return load_jsonstr(json_text.c_str());
}

int ecat_json::load_jsonstr(const char *ptr) {
    json eni_json;
    try {
        eni_json = json::parse(ptr);
    } catch (exception &e) {
        std::cout << e.what() << '\n';
        return ZMC_ERR_ENI_JSON_INVALID;
    }

    if (!eni_json.is_object()) {
        printf("json data error !! \r\n");
        return ZMC_ERR_ENI_JSON_INVALID;
    }

    try {
        // try to use an invalid array index
        eni_json.at("/slaves"_json_pointer);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
    }
    int vers = 0;
    try {
        auto ver = eni_json["eni_version"].get<std::string>();
        if (ver == "2.0.0") vers = 2;
    } catch (json::exception &e) {
    }
    const std::string KEYS[] = {"cycle_time", "mailbox_cycle_time", "auto_recovery_timeout"};
    uint32_t *pVal[] = {&master.cycle_time, &master.mailbox_cycle_time,
                        &master.auto_recovery_timeout};
    for (int i = 0; i < LEN(pVal); i++) {
        try {
            *pVal[i] = eni_json["master"]["base"][KEYS[i]].get<uint32_t>();
        } catch (json::exception &e) {
        }
    }
    {
        const std::string KEYS[] = {"di_begin_index", "do_begin_index", "ai_begin_index",
                                    "ao_begin_index"};
        uint32_t *pVal[] = {&master.di_begin_index, &master.do_begin_index, &master.ai_begin_index,
                            &master.ao_begin_index};
        for (int i = 0; i < LEN(pVal); i++) {
            try {
                *pVal[i] = eni_json["master"]["iomap"][KEYS[i]].get<uint32_t>();
            } catch (json::exception &e) {
                // cout << "not found [" << KEYS[i] << "], use default " << *pVal[i] << endl;
            }
        }
    }
    {
        const std::string KEYS[] = {"di_keys", "do_keys", "ai_keys", "ao_keys"};
        std::vector<uint32_t> *pVal[] = {&master.di_keys, &master.do_keys, &master.ai_keys,
                                         &master.ao_keys};
        for (int i = 0; i < LEN(pVal); i++) {
            try {
                std::vector<std::string> tmp =
                    eni_json["master"]["iomap"][KEYS[i]].get<std::vector<std::string>>();
                for (auto &v : tmp) {
                    pVal[i]->push_back(decHexStr2num(v.c_str()));
                }

            } catch (json::exception &e) {
                // cout << "not found [" << KEYS[i] << "], use empty " << endl;
            }
        }
        master.di_end = master.di_begin_index + master.di_keys.size();
        master.do_end = master.do_begin_index + master.do_keys.size();
        master.ai_end = master.ai_begin_index + master.ai_keys.size();
        master.ao_end = master.ao_begin_index + master.ao_keys.size();
    }
    slvInfo.slaves.resize(eni_json["slaves"].size());

    for (uint32_t i = 0; i < eni_json["slaves"].size(); i++) {
        const int ret = slave_info_push(slvInfo.slaves.at(i), eni_json["slaves"][i], vers);
        if (ret) return ret;
    }
    return 0;
}

int extract_eni_json_text(const char *eni_file, std::string &out_json) {
    out_json.clear();
    if (eni_file == NULL) {
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }

    FILE *fd = fopen(eni_file, "rb");
    if (fd == 0) {
        printf("open %s fail\r\n", eni_file);
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }

    if (fseek(fd, 0, SEEK_END) != 0) {
        fclose(fd);
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }
    const long eni_size_l = ftell(fd);
    if (eni_size_l < 0) {
        fclose(fd);
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }
    if (fseek(fd, 0, SEEK_SET) != 0) {
        fclose(fd);
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }

    const uint32_t eni_size = static_cast<uint32_t>(eni_size_l);
    if (eni_size > ENI_MAX_SIZE) {
        printf("ENI File Too Big \r\n");
        fclose(fd);
        return ZMC_ERR_ENI_FILE_TOO_LARGE;
    }

    std::vector<char> buff(static_cast<size_t>(eni_size) + 1u, 0);
    const size_t nread = fread(buff.data(), 1, eni_size, fd);
    fclose(fd);
    if (nread != eni_size) {
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }

    int ofs = 0;
    int length = 0;
    const int ret = ::extract_eni_json_slice(buff.data(), &ofs, &length);
    if (ret != 0) {
        return ret;
    }
    out_json.assign(buff.data() + ofs, static_cast<size_t>(length));
    return 0;
}

int ecat_json::pdo_obj_push(vector<pdo_info_t> &pdo_obj, const json &json_pdo, bool is_output,
                            uint8_t vers) {
    if (vers == 2)
        return _pdo_obj_push_ver2(pdo_obj, json_pdo, is_output);
    else
        return _pdo_obj_push_ver0(pdo_obj, json_pdo, is_output);
}
int ecat_json::_pdo_obj_push_ver0(vector<pdo_info_t> &pdo_obj, const json &json_pdo,
                                  bool is_output) {
    pdo_info_t pdo_info;
    string temp;

    for (uint32_t i = 0; i < json_pdo.size(); i++) {
        pdo_info.channel = ECAT_VALUE_ADD(json_pdo[i], "channel");
        pdo_info.name = ECAT_VALUE_ADD(json_pdo[i], "name");
        pdo_info.type = ECAT_VALUE_ADD(json_pdo[i], "type");

        temp = ECAT_VALUE_ADD(json_pdo[i], "index");
        pdo_info.index = decHexStr2num(temp.c_str());
        temp = ECAT_VALUE_ADD(json_pdo[i], "subindex");
        pdo_info.subindex = decHexStr2num(temp.c_str());

        temp = ECAT_VALUE_ADD(json_pdo[i], "bit_size");
        pdo_info.bit_size = decHexStr2num(temp.c_str());

        if (!xml_file_select) {
            temp = ECAT_VALUE_ADD(json_pdo[i], "bit_offset");
            pdo_info.bit_offset = decHexStr2num(temp.c_str());
        } else {
            if (is_output) {
                pdo_info.bit_offset = outputs_offset;
                outputs_offset += pdo_info.bit_size;
            } else {
                pdo_info.bit_offset = inputs_offset;
                inputs_offset += pdo_info.bit_size;
            }
        }
        pdo_obj.push_back(pdo_info);
    }
    return 0;
}
int ecat_json::_pdo_obj_push_ver2(vector<pdo_info_t> &pdo_obj, const json &json_pdo,
                                  bool is_output) {
    pdo_info_t pdo_info;
    string temp;

    for (uint32_t i = 0; i < json_pdo.size(); i++) {
        pdo_info.channel = ECAT_VALUE_ADD(json_pdo[i], "channel");
        pdo_info.name = ECAT_VALUE_ADD(json_pdo[i], "name");
        pdo_info.type = ECAT_VALUE_ADD(json_pdo[i], "type");

        pdo_info.index = json_pdo[i]["index"].get<uint32_t>();
        pdo_info.subindex = json_pdo[i]["subindex"].get<uint32_t>();
        pdo_info.bit_size = json_pdo[i]["bit_size"].get<uint32_t>();

        if (!xml_file_select) {
            pdo_info.bit_offset = json_pdo[i]["bit_offset"].get<uint32_t>();
        } else {
            if (is_output) {
                pdo_info.bit_offset = outputs_offset;
                outputs_offset += pdo_info.bit_size;
            } else {
                pdo_info.bit_offset = inputs_offset;
                inputs_offset += pdo_info.bit_size;
            }
        }
        pdo_obj.push_back(pdo_info);
    }
    return 0;
}

int ecat_json::slave_info_push(SLAVES_INFO_t &slave, const json &json_slaves, uint8_t vers) {
    json pdo;
    try {
        slave.name = ECAT_VALUE_ADD(json_slaves, "name");
        slave.vendor_id = json_text_or_number(json_slaves, "vendor_id");
        slave.vendor_name = ECAT_VALUE_ADD(json_slaves, "vendor_name");
        slave.type = ECAT_VALUE_ADD(json_slaves, "type");
        slave.product_code = json_text_or_number(json_slaves, "product_code");
        slave.revision_no = json_text_or_number(json_slaves, "revision_no");
        slave.product_revision = ECAT_VALUE_ADD(json_slaves, "product_revision");
// If it's an int, store it directly; if it's a string, convert it to int
// Define a macro to adapt assignment for phys_addr and auto_inc_addr (both string and integer
// types supported)
#define ECAT_JSON_ASSIGN_UINT32_FIELD(field, j, key)                                       \
    if ((j).contains(key)) {                                                               \
        if ((j)[key].is_number_integer()) {                                                \
            field = (j)[key].get<uint32_t>();                                              \
        } else if ((j)[key].is_string()) {                                                 \
            field = static_cast<uint32_t>(std::stoul((j)[key].get<string>(), nullptr, 0)); \
        } else {                                                                           \
            field = 0;                                                                     \
        }                                                                                  \
    } else {                                                                               \
        field = 0;                                                                         \
    }

        ECAT_JSON_ASSIGN_UINT32_FIELD(slave.phys_addr, json_slaves, "phys_addr")
        ECAT_JSON_ASSIGN_UINT32_FIELD(slave.auto_inc_addr, json_slaves, "auto_inc_addr")
#undef ECAT_JSON_ASSIGN_UINT32_FIELD
        slave.slot_name = ECAT_VALUE_ADD(json_slaves, "slot_name");
    } catch (json::exception &) {
        return ZMC_ERR_ENI_JSON_INVALID;
    }
    try {
        json_slaves.at("/outputs"_json_pointer);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
    }
    try {
        pdo = json_slaves.at("outputs");
        pdo_obj_push(slave.outputs, pdo, true, vers);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
    }
    try {
        json_slaves.at("/inputs"_json_pointer);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
    }
    try {
        pdo = json_slaves.at("inputs");
        pdo_obj_push(slave.inputs, pdo, false, vers);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
    }
    return 0;
}
} /* namespace zmc */
