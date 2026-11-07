#include "ServoFakFileState.h"

#include <map>
#include <string>

#include "ServoFakAlloc.hpp"
#include "common/thirdlib/nlohmann/json.hpp"
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


namespace zmc {

namespace {

using json = nlohmann::json;

static const char* kEnvStateFile = "ZECM_SERVO_FAK_STATE_FILE";
static const char* kEnvEnvFile = "ZECM_SERVO_FAK_ENV_FILE";
static const char* kDefaultEnvFileName = "zecm_servo_fak.env";
static const int kSnapshotVersion = 1;
static json g_extra = json::object();
static int g_tick = 0;
static std::string g_state_path_cache;
static bool g_state_path_resolved = false;

static bool is_comment_or_blank(const char* line);

static void trim_inplace(std::string& s) {
    size_t b = 0;
    while (b < s.size() && isspace((unsigned char)s[b])) {
        ++b;
    }
    size_t e = s.size();
    while (e > b && isspace((unsigned char)s[e - 1])) {
        --e;
    }
    s = s.substr(b, e - b);
}

static void strip_optional_quotes(std::string& s) {
    if (s.size() >= 2 &&
        ((s[0] == '"' && s[s.size() - 1] == '"') || (s[0] == '\'' && s[s.size() - 1] == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
}

static bool parse_env_line(const char* line, std::string* key, std::string* value) {
    if (!line || !key || !value) {
        return false;
    }
    const char* eq = strchr(line, '=');
    if (!eq) {
        return false;
    }
    *key = std::string(line, eq - line);
    *value = std::string(eq + 1);
    trim_inplace(*key);
    trim_inplace(*value);
    strip_optional_quotes(*value);
    return !key->empty();
}

static bool load_env_file(const char* path, std::map<std::string, std::string>& out) {
    if (!path || path[0] == '\0') {
        return false;
    }
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }
    char buf[4096];
    while (fgets(buf, (int)sizeof(buf), fp)) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[--len] = '\0';
        }
        if (is_comment_or_blank(buf)) {
            continue;
        }
        std::string key;
        std::string value;
        if (!parse_env_line(buf, &key, &value)) {
            printf("[ServoFakFileState] env: skip invalid line in %s\n", path);
            continue;
        }
        out[key] = value;
    }
    fclose(fp);
    return true;
}

static const char* resolve_state_file_path() {
    if (g_state_path_resolved) {
        return g_state_path_cache.empty() ? nullptr : g_state_path_cache.c_str();
    }
    g_state_path_resolved = true;

    const char* direct = getenv(kEnvStateFile);
    if (direct && direct[0] != '\0') {
        g_state_path_cache = direct;
        return g_state_path_cache.c_str();
    }

    std::map<std::string, std::string> envMap;
    const char* envFile = getenv(kEnvEnvFile);
    if (envFile && envFile[0] != '\0') {
        load_env_file(envFile, envMap);
    } else {
        load_env_file(kDefaultEnvFileName, envMap);
    }

    std::map<std::string, std::string>::const_iterator it = envMap.find(kEnvStateFile);
    if (it != envMap.end() && !it->second.empty()) {
        g_state_path_cache = it->second;
        return g_state_path_cache.c_str();
    }
    return nullptr;
}

static bool is_comment_or_blank(const char* line) {
    if (!line) {
        return true;
    }
    while (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n') {
        ++line;
    }
    return *line == '\0' || *line == '#';
}

static int parse_axis_index(const char* key, int* outIndex) {
    if (!key || !outIndex) {
        return -1;
    }
    char* end = nullptr;
    long v = strtol(key, &end, 10);
    if (end == key || *end != '\0' || v < 0) {
        return -1;
    }
    *outIndex = (int)v;
    return 0;
}

struct ServoSnapFields {
    int pos;
    int vel;
    int statusword;
    int err;
    int opmode;
};

static ServoSnapFields read_servo_fields(const json& node) {
    ServoSnapFields s = {0, 0, 0, 0, 0};
    if (!node.is_object()) {
        return s;
    }
    if (node.contains("pos")) {
        s.pos = node["pos"].get<int>();
    }
    if (node.contains("vel")) {
        s.vel = node["vel"].get<int>();
    }
    if (node.contains("statusword")) {
        s.statusword = node["statusword"].get<int>();
    }
    if (node.contains("err")) {
        s.err = node["err"].get<int>();
    }
    if (node.contains("opmode")) {
        s.opmode = node["opmode"].get<int>();
    }
    return s;
}

static void apply_servo_snapshot(ServoSgFaker& sv, const ServoSnapFields& snap,
                                 double cycleTimeSec) {
    ServoFakerToyCfg cfg;
    cfg.cycleTime = cycleTimeSec;
    cfg.posInit = snap.pos;
    cfg.decel = 10000.;
    sv.set_config(cfg);
    sv.diSetPosition = snap.pos;
    sv.diActVelocity = snap.vel;
    sv.wStatusword = (uint16_t)snap.statusword;
    sv.errCode = (uint16_t)snap.err;
    sv.byOpModeDisplay = (uint8_t)snap.opmode;
    sv.byOpMode = (uint8_t)snap.opmode;
}

static json capture_servo_node(const ServoSgFaker& sv) {
    json node = json::object();
    node["pos"] = sv.diActPosition;
    node["vel"] = sv.diActVelocity;
    node["statusword"] = sv.wStatusword;
    node["err"] = sv.errCode;
    node["opmode"] = sv.byOpModeDisplay;
    return node;
}

static bool parse_snapshot_line(const char* line, json* outRoot) {
    if (!line || !outRoot) {
        return false;
    }
    try {
        json root = json::parse(line);
        if (!root.is_object()) {
            return false;
        }
        if (!root.contains("version") || root["version"].get<int>() != kSnapshotVersion) {
            return false;
        }
        *outRoot = root;
        return true;
    } catch (...) {
        return false;
    }
}

static bool read_last_snapshot(const char* path, json* outRoot) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }
    char buf[65536];
    json last;
    bool found = false;
    while (fgets(buf, (int)sizeof(buf), fp)) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[--len] = '\0';
        }
        if (is_comment_or_blank(buf)) {
            continue;
        }
        json root;
        if (parse_snapshot_line(buf, &root)) {
            last = root;
            found = true;
        }
    }
    fclose(fp);
    if (!found) {
        return false;
    }
    *outRoot = last;
    return true;
}

}  // namespace

bool servo_fak_state_enabled() { return resolve_state_file_path() != nullptr; }

void servo_fak_state_set_extra_string(const char* key, const char* value) {
    if (!key || !value) {
        return;
    }
    g_extra[key] = value;
}

void servo_fak_state_on_cycle() {
    if (!servo_fak_state_enabled()) {
        return;
    }
    ++g_tick;
}

int servo_fak_state_load(ServoSgFaker* svArr, int count, double cycleTimeSec) {
    const char* path = resolve_state_file_path();
    if (!path || !svArr || count <= 0) {
        return -1;
    }

    json root;
    if (!read_last_snapshot(path, &root)) {
        printf("[ServoFakFileState] load skipped: no snapshot in %s\n", path);
        return -1;
    }

    if (root.contains("extra") && root["extra"].is_object()) {
        g_extra = root["extra"];
    } else {
        g_extra = json::object();
    }

    if (!root.contains("servos") || !root["servos"].is_object()) {
        printf("[ServoFakFileState] load: missing servos object\n");
        return -1;
    }

    if (root.contains("tick")) {
        g_tick = root["tick"].get<int>();
    } else {
        g_tick = 0;
    }

    const json& servos = root["servos"];
    for (json::const_iterator it = servos.begin(); it != servos.end(); ++it) {
        int idx = -1;
        if (parse_axis_index(it.key().c_str(), &idx) != 0) {
            printf("[ServoFakFileState] load: ignore invalid axis key %s\n", it.key().c_str());
            continue;
        }
        if (idx >= count) {
            printf("[ServoFakFileState] load: axis %d out of range (count=%d)\n", idx, count);
            continue;
        }
        ServoSnapFields snap = read_servo_fields(it.value());
        apply_servo_snapshot(svArr[idx], snap, cycleTimeSec);
    }

    printf("[ServoFakFileState] loaded snapshot from %s\n", path);
    return 0;
}

int servo_fak_state_save(const ServoSgFaker* svArr, int count) {
    const char* path = resolve_state_file_path();
    if (!path || !svArr || count <= 0) {
        return -1;
    }

    json root = json::object();
    root["version"] = kSnapshotVersion;
    root["tick"] = g_tick;
    root["servos"] = json::object();
    for (int i = 0; i < count; ++i) {
        char key[16];
        snprintf(key, sizeof(key), "%d", i);
        root["servos"][key] = capture_servo_node(svArr[i]);
    }
    root["extra"] = g_extra.is_object() ? g_extra : json::object();

    FILE* fp = fopen(path, "ab");
    if (!fp) {
        printf("[ServoFakFileState] save failed: cannot open %s\n", path);
        return -1;
    }

    std::string line = root.dump();
    line.push_back('\n');
    size_t written = fwrite(line.c_str(), 1, line.size(), fp);
    fclose(fp);
    if (written != line.size()) {
        printf("[ServoFakFileState] save failed: short write %s\n", path);
        return -1;
    }

    printf("[ServoFakFileState] appended snapshot to %s\n", path);
    return 0;
}

}  // namespace zmc
