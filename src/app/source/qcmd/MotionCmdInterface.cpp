#include "qcmd/MotionCmdInterface.hpp"

#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

#include "common/str2hash.hpp"
#include "common/utils/double2frac.hpp"
#include "common/zmc/zmc_predef.h"
#include "fbt_errorcode_def.h"

namespace zmc {

namespace {

inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }

static void parseIntList(const char* args, std::vector<int>& out) {
    out.clear();
    if (!args) return;
    const char* p = args;
    while (*p) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) break;
        char* end = nullptr;
        long v = std::strtol(p, &end, 10);
        if (end == p) break;
        out.push_back(static_cast<int>(v));
        p = end;
    }
}

static void parseDoubleList(const char* args, std::vector<double>& out) {
    out.clear();
    if (!args) return;
    const char* p = args;
    while (*p) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) break;
        char* end = nullptr;
        double v = std::strtod(p, &end);
        if (end == p) break;
        out.push_back(v);
        p = end;
    }
}

// Parses exactly expectedCount finite double values separated by whitespace.
static bool parseFixedDoubleList(const char* args, size_t expectedCount,
                                 std::vector<double>& out) {
    out.clear();
    if (!args) return false;

    const char* p = args;
    while (*p) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) break;

        char* end = nullptr;
        const double value = std::strtod(p, &end);
        if (end == p || !std::isfinite(value)) return false;
        out.push_back(value);
        p = end;
    }
    return out.size() == expectedCount;
}

// Parses exactly expectedCount unsigned 32-bit integers in decimal or hexadecimal form.
static bool parseFixedUint32List(const char* args, size_t expectedCount,
                                 std::vector<uint32_t>& out) {
    out.clear();
    if (!args) return false;

    const char* p = args;
    while (*p) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) break;
        if (*p == '-') return false;

        errno = 0;
        char* end = nullptr;
        const unsigned long value = std::strtoul(p, &end, 0);
        if (end == p || errno == ERANGE ||
            value > static_cast<unsigned long>(std::numeric_limits<uint32_t>::max())) {
            return false;
        }
        out.push_back(static_cast<uint32_t>(value));
        p = end;
    }
    return out.size() == expectedCount;
}

static inline bool isSingleAxisCommand(unsigned int commandHash) {
    switch (commandHash) {
        case strhash("axis_enable"):
        case strhash("poweron"):
        case strhash("poweroff"):
        case strhash("reset"):
        case strhash("movesgabs"):
        case strhash("movesg"):
        case strhash("movesgrel"):
        case strhash("moveadd"):
        case strhash("home"):
        case strhash("datum"):
        case strhash("vmove"):
        case strhash("forward"):
        case strhash("reverse"):
        case strhash("halt"):
        case strhash("stop"):
        case strhash("gearin"):
        case strhash("gearout"):
        case strhash("units"):
        case strhash("speed"):
        case strhash("accel"):
        case strhash("decel"):
        case strhash("sparam"):
        case strhash("fastdecel"):
        case strhash("speedratio"):
        case strhash("set_speed_ratio"):
        case strhash("buffermode"):
        case strhash("defpos"):
            return true;
        default:
            return false;
    }
}

static bool splitAxisSuffix(const char* args, int& axis, std::string& commandArgs) {
    axis = 0;
    commandArgs = args ? args : "";
    if (commandArgs.empty()) return true;

    const size_t valueEnd = commandArgs.find_last_not_of(" \t");
    const size_t valueStart = commandArgs.find_last_of(" \t", valueEnd) + 1;
    const size_t keywordEnd = commandArgs.find_last_not_of(" \t", valueStart - 1);
    if (keywordEnd == std::string::npos) return true;
    const size_t keywordStart = commandArgs.find_last_of(" \t", keywordEnd) + 1;
    std::string keyword = commandArgs.substr(keywordStart, keywordEnd - keywordStart + 1);
    for (size_t i = 0; i < keyword.size(); ++i) {
        keyword[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(keyword[i])));
    }

    if (keyword != "axis") {
        if (commandArgs.substr(valueStart, valueEnd - valueStart + 1) == "axis") return false;
        return true;
    }

    const std::string valueToken = commandArgs.substr(valueStart, valueEnd - valueStart + 1);
    errno = 0;
    char* parseEnd = nullptr;
    const long value = std::strtol(valueToken.c_str(), &parseEnd, 10);
    if (parseEnd == valueToken.c_str() || *parseEnd != '\0' || errno == ERANGE ||
        value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
        return false;
    }

    axis = static_cast<int>(value);
    commandArgs.resize(keywordStart);
    while (!commandArgs.empty() &&
           std::isspace(static_cast<unsigned char>(commandArgs[commandArgs.size() - 1]))) {
        commandArgs.resize(commandArgs.size() - 1);
    }
    return true;
}

static inline bool isAxisGroupCommand(unsigned int commandHash) {
    switch (commandHash) {
        case strhash("grp_en"):
        case strhash("grp_dis"):
        case strhash("grp_rst"):
        case strhash("moveabs"):
        case strhash("move"):
        case strhash("moverel"):
        case strhash("movecirc"):
        case strhash("movecircabs"):
        case strhash("movecirc2"):
        case strhash("movecirc2abs"):
            return true;
        default:
            return false;
    }
}

static bool parseAxisIndexList(const std::string& text, std::vector<int>& axes) {
    axes.clear();
    const char* p = text.c_str();
    while (*p) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) break;
        errno = 0;
        char* end = nullptr;
        const long value = std::strtol(p, &end, 10);
        if (end == p || errno == ERANGE || value < std::numeric_limits<int>::min() ||
            value > std::numeric_limits<int>::max() ||
            (*end && !std::isspace(static_cast<unsigned char>(*end)))) {
            return false;
        }
        axes.push_back(static_cast<int>(value));
        p = end;
    }
    return !axes.empty();
}

static bool splitAxisGroupSuffix(const char* args, std::string& commandArgs,
                                 std::vector<int>& axes, bool& hasSuffix) {
    commandArgs = args ? args : "";
    axes.clear();
    hasSuffix = false;

    size_t tokenStart = 0;
    while (tokenStart < commandArgs.size()) {
        while (tokenStart < commandArgs.size() &&
               std::isspace(static_cast<unsigned char>(commandArgs[tokenStart]))) {
            ++tokenStart;
        }
        if (tokenStart == commandArgs.size()) break;
        size_t tokenEnd = tokenStart;
        while (tokenEnd < commandArgs.size() &&
               !std::isspace(static_cast<unsigned char>(commandArgs[tokenEnd]))) {
            ++tokenEnd;
        }
        std::string token = commandArgs.substr(tokenStart, tokenEnd - tokenStart);
        for (size_t i = 0; i < token.size(); ++i) {
            token[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(token[i])));
        }
        if (token == "axgrp") {
            hasSuffix = true;
            const std::string axisList = commandArgs.substr(tokenEnd);
            commandArgs.resize(tokenStart);
            while (!commandArgs.empty() &&
                   std::isspace(static_cast<unsigned char>(commandArgs[commandArgs.size() - 1]))) {
                commandArgs.resize(commandArgs.size() - 1);
            }
            return parseAxisIndexList(axisList, axes);
        }
        tokenStart = tokenEnd;
    }
    return true;
}

static void makeDefaultAxisGroup(size_t count, std::vector<int>& axes) {
    axes.resize(count);
    for (size_t i = 0; i < count; ++i) axes[i] = static_cast<int>(i);
}

// Check whether a parsed IO number is within the configured logical-port capacity.
static bool isWireIoNumber(uint32_t ioNumber) {
    return ioNumber < static_cast<uint32_t>(MC_IO_MAX_COUNT);
}

// Helper function: split multi-line command into lines
static void splitLines(const char* cmd, std::vector<std::string>& lines) {
    if (!cmd || !cmd[0]) return;

    const char* start = cmd;
    const char* p = cmd;

    while (*p) {
        if (*p == '\n' || (*p == '\r' && p[1] == '\n')) {
            size_t len = p - start;
            if (len > 0) {
                lines.push_back(std::string(start, len));
            }
            p = (*p == '\r') ? p + 2 : p + 1;
            start = p;
        } else {
            p++;
        }
    }

    if (p > start) {
        lines.push_back(std::string(start, p - start));
    }
}

}  // namespace

// Compile-time parse: typed items enqueue via appendOrDefer / priority / wait paths.
// Never stores raw strings in scriptQueue_; busy does not reject (tail-merge when possible).
int MotionCmdHerit::executeCommand(const char* cmd) {
    if (!cmd || !cmd[0]) {
        return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_EMPTY);
    }

    std::vector<std::string> lines;
    splitLines(cmd, lines);
    if (lines.empty()) {
        return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_EMPTY);
    }

    for (const auto& line : lines) {
        if (line.empty()) {
            continue;
        }
        const int ret = executeSingleLineCommand(line.c_str());
        if (ret != 0) {
            return ret;
        }
    }
    return motcmdRet(ZMC_NO_ERROR);
}

// Execute single line command (helper function)
int MotionCmdHerit::executeSingleLineCommand(const char* cmd) {
    if (!cmd || !cmd[0]) {
        return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_LINE_EMPTY);
    }

    // Parse command line: split by whitespace
    const char* p = cmd;
    while (*p && (*p == ' ' || *p == '\t')) {
        p++;  // Skip leading whitespace
    }
    if (!*p) {
        return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_LINE_EMPTY);
    }

    // Extract command name
    const char* cmdStart = p;
    while (*p && *p != ' ' && *p != '\t') {
        p++;
    }
    size_t cmdLen = p - cmdStart;
    if (cmdLen == 0) {
        return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_LINE_EMPTY);
    }

    // Skip whitespace before arguments
    while (*p && (*p == ' ' || *p == '\t')) {
        p++;
    }
    const char* args = (*p) ? p : nullptr;

    // Compare command name (case-insensitive)
    std::string cmdName(cmdStart, cmdLen);
    for (char& c : cmdName) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    const unsigned int commandHash = strhash(cmdName.c_str());

    int axis = 0;
    std::string axisCommandArgs;
    if (isSingleAxisCommand(commandHash)) {
        if (!splitAxisSuffix(args, axis, axisCommandArgs)) {
            return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
        }
        args = axisCommandArgs.empty() ? nullptr : axisCommandArgs.c_str();
    }
    std::vector<int> commandAxisGroup;
    bool hasAxisGroupSuffix = false;
    std::string axisGroupCommandArgs;
    if (isAxisGroupCommand(commandHash)) {
        if (!splitAxisGroupSuffix(args, axisGroupCommandArgs, commandAxisGroup,
                                  hasAxisGroupSuffix)) {
            return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
        }
        args = axisGroupCommandArgs.empty() ? nullptr : axisGroupCommandArgs.c_str();
    }

    switch (commandHash) {
        case strhash("base"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return setBaseAxis(std::atoi(args));
        }
        case strhash("axis"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return setAxis(std::atoi(args));
        }
        case strhash("read_di"):
        case strhash("read_do"):
        case strhash("read_ai"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<uint32_t> values;
            if (!parseFixedUint32List(args, 1, values) || !isWireIoNumber(values[0])) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            switch (commandHash) {
                case strhash("read_di"):
                    return requestReadDi(values[0]);
                case strhash("read_do"):
                    return requestReadDo(values[0]);
                default:
                    return requestReadAi(values[0]);
            }
        }
        case strhash("read_di_multi"):
        case strhash("read_do_multi"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<uint32_t> values;
            if (!parseFixedUint32List(args, 2, values) || !isWireIoNumber(values[0]) ||
                !isWireIoNumber(values[1])) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            switch (commandHash) {
                case strhash("read_di_multi"):
                    return requestReadDiMulti(values[0], values[1]);
                default:
                    return requestReadDoMulti(values[0], values[1]);
            }
        }
        case strhash("write_do"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<uint32_t> values;
            if (!parseFixedUint32List(args, 2, values) || !isWireIoNumber(values[0]) ||
                values[1] > 1u) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            return writeDo(values[0], values[1] != 0u);
        }
        case strhash("write_do_multi"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<uint32_t> values;
            if (!parseFixedUint32List(args, 3, values) || !isWireIoNumber(values[0]) ||
                !isWireIoNumber(values[1])) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            return writeDoMulti(values[0], values[1], values[2]);
        }
        case strhash("write_ao"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<uint32_t> values;
            if (!parseFixedUint32List(args, 2, values) || !isWireIoNumber(values[0])) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            return writeAo(values[0], values[1]);
        }
        case strhash("axis_enable"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return axisEnable(axis, std::atoi(args) != 0);
        }
        case strhash("poweron"):
            return powerOn(axis);
        case strhash("poweroff"):
            return powerOff(axis);
        case strhash("reset"):
            return reset(axis);
        case strhash("movesgabs"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return moveAbs(axis, std::atof(args));
        }
        case strhash("movesg"):
        case strhash("movesgrel"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return moveRel(axis, std::atof(args));
        }
        case strhash("moveadd"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return moveAdd(axis, std::atof(args));
        }
        case strhash("home"):
            return home(axis, args ? std::atof(args) : 0.0);
        case strhash("datum"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return datum(axis, std::atoi(args));
        }
        case strhash("vmove"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return moveVelocity(axis, std::atoi(args));
        }
        case strhash("forward"):
            return forward(axis);
        case strhash("reverse"):
            return reverse(axis);
        case strhash("halt"):
            return halt(axis);
        case strhash("stop"):
            return stop(axis);
        case strhash("gearin"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            setMasterAxis(axis, std::atoi(args));
            return gearIn(axis);
        }
        case strhash("gearout"):
            return gearOut(axis);
        case strhash("units"):
        case strhash("speed"):
        case strhash("accel"):
        case strhash("decel"):
        case strhash("sparam"):
        case strhash("fastdecel"):
        case strhash("speedratio"):
        case strhash("set_speed_ratio"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            const double value = std::atof(args);
            switch (commandHash) {
                // The legacy script keyword keeps fractional double input; only the
                // typed RPC/HTTP API carries num/den directly. GLM-5.3 (opencode).
                case strhash("units"): {
                    NumDenUnion numDen = {{1, 1}};
                    doubleToFraction(value, numDen.num, numDen.den);
                    return setUnitsNumDen(axis, numDen.num, numDen.den);
                }
                case strhash("speed"):
                    return setSpeed(axis, value);
                case strhash("accel"):
                    return setAccel(axis, value);
                case strhash("decel"):
                    return setDecel(axis, value);
                case strhash("sparam"):
                    return setJerk(axis, value);
                case strhash("fastdecel"):
                    return setFastDecel(axis, value);
                default:
                    return setSpeedRatio(axis, value);
            }
        }
        case strhash("buffermode"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return setBufferMode(axis, std::atoi(args));
        }
        case strhash("group"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<int> axisList;
            parseIntList(args, axisList);
            if (axisList.empty()) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            const int ret = groupSetAxes(static_cast<int>(axisList.size()), axisList.data());
            if (ret == 0) scriptGroupAxisIndices_ = axisList;
            return ret;
        }
        case strhash("group_select"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return groupSelect(std::atoi(args));
        }
        case strhash("grp_en"):
        case strhash("grp_dis"):
        case strhash("grp_rst"): {
            const std::vector<int>& axes =
                hasAxisGroupSuffix ? commandAxisGroup : scriptGroupAxisIndices_;
            if (axes.empty()) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            }
            switch (commandHash) {
                case strhash("grp_en"):
                    return groupEnable(static_cast<int>(axes.size()), axes.data());
                case strhash("grp_dis"):
                    return groupDisable(static_cast<int>(axes.size()), axes.data());
                default:
                    return groupReset(static_cast<int>(axes.size()), axes.data());
            }
        }
        case strhash("moveabs"):
        case strhash("move"):
        case strhash("moverel"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            std::vector<double> posList;
            parseDoubleList(args, posList);
            if (posList.empty()) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            std::vector<int> axes =
                hasAxisGroupSuffix ? commandAxisGroup : scriptGroupAxisIndices_;
            if (axes.empty()) makeDefaultAxisGroup(posList.size(), axes);
            if (axes.size() != posList.size()) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            const int axisCount = static_cast<int>(axes.size());
            switch (commandHash) {
                case strhash("moveabs"):
                    return multiMoveAbs(axisCount, axes.data(), posList.data());
                default:
                    return multiMoveRel(axisCount, axes.data(), posList.data());
            }
        }
        case strhash("movecirc"):
        case strhash("movecircabs"):
        case strhash("movecirc2"):
        case strhash("movecirc2abs"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            bool centerDefined = false;
            switch (commandHash) {
                case strhash("movecirc"):
                case strhash("movecircabs"):
                    centerDefined = true;
                    break;
                default:
                    break;
            }
            std::vector<double> values;
            const size_t valueCount = centerDefined ? 5 : 4;
            if (!parseFixedDoubleList(args, valueCount, values) ||
                (centerDefined && values[4] != 0.0 && values[4] != 1.0)) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            std::vector<int> axes =
                hasAxisGroupSuffix ? commandAxisGroup : scriptGroupAxisIndices_;
            if (axes.empty()) makeDefaultAxisGroup(2, axes);
            if (axes.size() != 2) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            const int axisCount = static_cast<int>(axes.size());
            switch (commandHash) {
                case strhash("movecirc"):
                    return moveCirc(axisCount, axes.data(), values[0], values[1], values[2],
                                    values[3], static_cast<int>(values[4]));
                case strhash("movecircabs"):
                    return moveCircAbs(axisCount, axes.data(), values[0], values[1], values[2],
                                       values[3], static_cast<int>(values[4]));
                case strhash("movecirc2"):
                    return moveCirc2(axisCount, axes.data(), values[0], values[1], values[2],
                                     values[3]);
                default:
                    return moveCirc2Abs(axisCount, axes.data(), values[0], values[1], values[2],
                                        values[3]);
            }
        }
        case strhash("defpos"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return setPosition(axis, std::atof(args));
        }
        case strhash("wait_tick"):
        case strhash("wait_ticks"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return waitTicks(static_cast<uint32_t>(std::strtoul(args, nullptr, 10)));
        }
        case strhash("wait_time"):
        case strhash("wait_seconds"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            return waitSeconds(std::atof(args));
        }
        case strhash("wait_idle"): {
            const int waitAxis = args ? std::atoi(args) : 0;
            return waitAxisIdle(waitAxis);
        }
        case strhash("autoflush"): {
            if (!args) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG);
            setAutoFlushEnabled(std::atoi(args) != 0);
            return 0;
        }
        default:
            return motcmdRet(ZMC_ERR_MOTCMD_UNKNOWN_COMMAND);
    }
}
int MotionCmdHerit::runOnce() {
    const int ret = heartbeat();
    processSyncRequests();
    return ret;
}
}  // namespace zmc
