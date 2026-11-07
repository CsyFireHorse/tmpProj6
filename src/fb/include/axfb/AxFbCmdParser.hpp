#include <fstream>
#include <string>

#include "AxFbCmdManager.h"
#include "common/thirdlib/nlohmann/json.hpp"
#include "pou/AxisJsonLoader.h"
#include "str2axFbAttr.hpp"

using namespace zmc;

// Returns the path-string without the filename part
inline std::string getFolderPath(const std::string& path) {
    // size_t found = path.find_last_of(get_spliter());  // "/\\"
    const size_t found = path.find_last_of("/\\");
    return (found == std::string::npos) ? "." : path.substr(0, found);
}

inline int load_AxFbCmdConfig_from_json(AxFbCmdConfig& config, const std::string& filename) {
    std::ifstream f(filename);
    if (!f) return -1;

    nlohmann::json js;
    f >> js;

    auto& jRun = js["run_config"];
    config.runCfg = RunConfig{jRun.value("cycle_time", 0.001), jRun.value("exit_count", 0),
                              jRun.value("verbose", 0), (uint16_t)jRun.value("verbose_modulo", 0)};

    for (auto& jType : js["axis_types"]) {
        config.axTypes.push_back(StringToAxFactoryType(jType.get<std::string>().c_str()));
    }
    if (js.contains("axis_config")) {
        for (auto& jType : js["axis_config"]) {
            AxisCiaConfig* pCfg = new AxisCiaConfig;
            AxisCiaConfig_default_init(pCfg);
            auto jsonFile = jType.get<std::string>();
            if (jsonFile.rfind("./", 0) == 0) {
                const auto& dirname = getFolderPath(filename);
                jsonFile = dirname + "/" + jsonFile.substr(2);
            }
            // printf("jsonfile=%s\n", jsonFile.c_str());
            int ret = load_axis_json(jsonFile.c_str(), *pCfg);
            config.axCfgs.push_back(pCfg);
        }
    }

    for (auto& jFb : js["fb_types"]) {
        config.fbTypes.push_back(StringToFbTypeEnum(jFb.get<std::string>().c_str()));
    }

    if (js.contains("commands")) {
        for (auto& jCmd : js["commands"]) {
            // jCmd["tick"].get<int>(); //
            uint8_t action = 0, attr = 0;
            // try {
            // auto v = jCmd["tick"].get<bool>();
            if (jCmd.contains("action"))
                action =
                    (uint8_t)StringToAxFbActionType(jCmd["action"].get<std::string>().c_str());
            if (jCmd.contains("attr"))
                attr = (uint8_t)StringToFbAttrEnum(jCmd["attr"].get<std::string>().c_str());
            config.cmds.push_back({(int16_t)jCmd.value("fbId", -1), attr, action,
                                   jCmd.value("tick", 0), jCmd.value("val", 0.0)});
        }
    }

    if (js.contains("vars")) {
        for (auto& jVar : js["vars"]) {
            VarDefCfg varCfg{strdup(jVar["name"].get<std::string>().c_str()),
                             static_cast<uint16_t>(jVar["size"].get<int>()),
                             static_cast<uint16_t>(jVar["type"].get<int>())};
            config.vars.push_back(varCfg);
        }
    }
    return 0;
}