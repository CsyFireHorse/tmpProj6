#include "memory/MemoryPouEngine.hpp"

#include <cctype>
#include <cstring>
#include <limits>

namespace zmc {

namespace {

std::string memory_pou_trim(const std::string& text) {
    std::size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }
    std::size_t last = text.size();
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }
    return text.substr(first, last - first);
}

std::size_t memory_pou_skip_leading_ws(const std::string& script, std::size_t pos) {
    while (pos < script.size() && std::isspace(static_cast<unsigned char>(script[pos]))) {
        ++pos;
    }
    return pos;
}

int32_t memory_pou_line_at(const std::string& script, std::size_t pos) {
    int32_t line = 1;
    for (std::size_t i = 0; i < pos && i < script.size(); ++i) {
        if (script[i] == '\n') {
            if (line == std::numeric_limits<int32_t>::max()) {
                return -1;
            }
            ++line;
        }
    }
    return line;
}

int32_t memory_pou_stmt_off(std::size_t pos) {
    if (pos > static_cast<std::size_t>(std::numeric_limits<int32_t>::max())) {
        return -1;
    }
    return static_cast<int32_t>(pos);
}

MemoryPouCmd memory_pou_make_cmd(uint8_t op) {
    MemoryPouCmd cmd;
    cmd.op = op;
    cmd.len = 0;
    cmd.cmp = 0;
    cmd._pad = 0;
    cmd.tick = 0;
    cmd.dst = 0;
    cmd.data = 0;
    cmd.line = -1;
    cmd.stmtOff = -1;
    return cmd;
}

bool memory_pou_parse_uint16(const std::string& statement, std::size_t& pos, uint16_t& value) {
    while (pos < statement.size() && std::isspace(static_cast<unsigned char>(statement[pos]))) {
        ++pos;
    }
    if (pos >= statement.size()) {
        return false;
    }
    const std::size_t numBegin = pos;
    unsigned long num = 0;
    while (pos < statement.size() && std::isdigit(static_cast<unsigned char>(statement[pos]))) {
        const unsigned int digit = static_cast<unsigned int>(statement[pos] - '0');
        if (num > (std::numeric_limits<uint16_t>::max() - digit) / 10) {
            return false;
        }
        num = num * 10 + digit;
        ++pos;
    }
    if (pos == numBegin) {
        return false;
    }
    while (pos < statement.size() && std::isspace(static_cast<unsigned char>(statement[pos]))) {
        ++pos;
    }
    if (pos != statement.size()) {
        return false;
    }
    value = static_cast<uint16_t>(num);
    return true;
}

bool memory_pou_starts_with_keyword(const std::string& statement, const char* keyword,
                                    std::size_t& keywordEnd) {
    const std::size_t len = std::strlen(keyword);
    if (statement.size() < len || statement.compare(0, len, keyword) != 0) {
        return false;
    }
    if (statement.size() > len) {
        const unsigned char next = static_cast<unsigned char>(statement[len]);
        if (!std::isspace(next)) {
            return false;
        }
    }
    keywordEnd = len;
    return true;
}

struct MemoryPouExpectOpSpec {
    const char* text;
    uint16_t cmp;
    std::size_t len;
};

bool memory_pou_parse_expect_body(const std::string& body, std::string& lhs, uint16_t& cmp,
                                  std::string& rhs) {
    static const MemoryPouExpectOpSpec kOps[] = {
        {">=", kMemoryPouCmpGe, 2}, {"<=", kMemoryPouCmpLe, 2}, {"==", kMemoryPouCmpEq, 2},
        {"!=", kMemoryPouCmpNe, 2}, {">", kMemoryPouCmpGt, 1},  {"<", kMemoryPouCmpLt, 1},
        {"=", kMemoryPouCmpEq, 1},
    };

    for (std::size_t i = 0; i < sizeof(kOps) / sizeof(kOps[0]); ++i) {
        const MemoryPouExpectOpSpec& spec = kOps[i];
        const std::size_t pos = body.find(spec.text);
        if (pos == std::string::npos) {
            continue;
        }
        lhs = memory_pou_trim(body.substr(0, pos));
        rhs = memory_pou_trim(body.substr(pos + spec.len));
        if (lhs.empty() || rhs.empty()) {
            return false;
        }
        cmp = spec.cmp;
        return true;
    }
    return false;
}

}  // namespace

MemoryPouEngine::MemoryPouEngine()
    : errorCode_(MEMOP_OK),
      cmdIndex_(0),
      tick_(0),
      waitUntilTick_(0),
      parseLine_(-1),
      parseStmtOff_(-1),
      parseCmp_(0) {}

int MemoryPouEngine::loadTypeLayout(const std::string& content) {
    return memoryOperator_.loadTypeLayoutContent(content);
}

int MemoryPouEngine::configure(const std::string& initConfig) {
    return memoryOperator_.configure(initConfig);
}

void MemoryPouEngine::bindMemory(void* base, std::size_t bytes) {
    memoryOperator_.bindMemory(base, bytes);
}

void MemoryPouEngine::cmdAdvance() {
    executeCommandsAtCurrentTick();
}

const MemoryPouCaptureInfo* MemoryPouEngine::captureAt(std::size_t index) const {
    if (index >= captureInfoVec_.size()) {
        return NULL;
    }
    return &captureInfoVec_[index];
}

const unsigned char* MemoryPouEngine::valueBufData() const {
    return valueBuf_.empty() ? NULL : &valueBuf_[0];
}

void MemoryPouEngine::executeCommandsAtCurrentTick() {
    if (errorCode_ != MEMOP_OK) {
        return;
    }
    if (tick_ < waitUntilTick_) {
        ++tick_;
        return;
    }
    while (cmdIndex_ < cmdQueue_.size()) {
        const MemoryPouCmd& cmd = cmdQueue_[cmdIndex_];
        if (cmd.op == kMemoryPouCmdTick || cmd.op == kMemoryPouCmdDelay) {
            waitUntilTick_ = (cmd.op == kMemoryPouCmdDelay) ? (tick_ + cmd.tick) : cmd.tick;
            ++cmdIndex_;
            if (tick_ < waitUntilTick_) {
                break;
            }
            continue;
        }
        const int rc = applyCmd(cmd);
        if (rc != MEMOP_OK) {
            errorCode_ = rc;
            return;
        }
        ++cmdIndex_;
    }
    ++tick_;
}

int MemoryPouEngine::appendCapture(const MemoryPouCmd& cmd, const unsigned char* bytes,
                                   MemoryPouCaptureKind kind) {
    if (!bytes || cmd.len == 0) {
        return MEMOP_ERR_ASSIGN;
    }
    MemoryPouCaptureInfo info;
    info.tick = tick_;
    info.line = cmd.line;
    info.stmtOff = cmd.stmtOff;
    info.src = cmd.dst;
    info.bufOff = static_cast<uint32_t>(valueBuf_.size());
    info.len = cmd.len;
    info.kind = static_cast<uint32_t>(kind);
    valueBuf_.insert(valueBuf_.end(), bytes, bytes + cmd.len);
    captureInfoVec_.push_back(info);
    return MEMOP_OK;
}

int MemoryPouEngine::applyCmd(const MemoryPouCmd& cmd) {
    if (cmd.op == kMemoryPouCmdAssign) {
        return memoryOperator_.assign(cmd.dst, &cmd.data, cmd.len);
    }
    if (cmd.op == kMemoryPouCmdCopy) {
        return memoryOperator_.copy(cmd.dst, static_cast<std::size_t>(cmd.data), cmd.len);
    }
    if (cmd.op == kMemoryPouCmdOut) {
        unsigned char actual[8];
        const int rc = memoryOperator_.read(cmd.dst, actual, cmd.len);
        if (rc != MEMOP_OK) {
            return rc;
        }
        return appendCapture(cmd, actual, kMemoryPouCaptureOut);
    }
    if (cmd.op == kMemoryPouCmdExpect) {
        unsigned char actual[8];
        const int rc = memoryOperator_.read(cmd.dst, actual, cmd.len);
        if (rc != MEMOP_OK) {
            return rc;
        }
        const int cmpRc =
            memoryOperator_.compare(cmd.dst, &cmd.data, cmd.len, static_cast<int>(cmd.cmp));
        const bool matched = (cmpRc == MEMOP_OK);
        const MemoryPouCaptureKind kind =
            matched ? kMemoryPouCaptureExpectOk : kMemoryPouCaptureExpectFail;
        const int capRc = appendCapture(cmd, actual, kind);
        if (capRc != MEMOP_OK) {
            return capRc;
        }
        return matched ? MEMOP_OK : MEMOP_ERR_EXPECT;
    }
    return MEMOP_ERR_ASSIGN;
}

int MemoryPouEngine::parseScheduledScript(const std::string& script) {
    if (memory_pou_trim(script).empty()) {
        return MEMOP_ERR_INIT;
    }
    cmdQueue_.clear();
    valueBuf_.clear();
    captureInfoVec_.clear();
    cmdIndex_ = 0;
    waitUntilTick_ = 0;
    tick_ = 0;
    errorCode_ = MEMOP_OK;
    parseLine_ = -1;
    parseStmtOff_ = -1;
    parseCmp_ = 0;

    std::size_t begin = 0;
    for (std::size_t pos = 0; pos <= script.size(); ++pos) {
        const bool atEnd = pos == script.size();
        if (!atEnd && script[pos] != ';' && script[pos] != '\n' && script[pos] != '\r') {
            continue;
        }
        const std::size_t stmtBegin = memory_pou_skip_leading_ws(script, begin);
        parseLine_ = memory_pou_line_at(script, stmtBegin);
        parseStmtOff_ = memory_pou_stmt_off(stmtBegin);
        const std::string statement = memory_pou_trim(script.substr(begin, pos - begin));
        if (!atEnd && script[pos] == '\r' && pos + 1 < script.size() && script[pos + 1] == '\n') {
            ++pos;
        }
        begin = pos + 1;
        if (statement.empty()) {
            continue;
        }
        const int rc = addScheduledStatement(statement);
        if (rc != MEMOP_OK) {
            return rc;
        }
    }
    parseLine_ = -1;
    parseStmtOff_ = -1;
    parseCmp_ = 0;
    return cmdQueue_.empty() ? MEMOP_ERR_INIT : MEMOP_OK;
}

int MemoryPouEngine::addScheduledStatement(const std::string& statement) {
    if (statement.empty()) {
        return MEMOP_ERR_INIT;
    }
    if (statement[0] == '@') {
        return MEMOP_ERR_INIT;
    }
    std::size_t keywordEnd = 0;
    if (memory_pou_starts_with_keyword(statement, "tick_to", keywordEnd)) {
        uint16_t tick = 0;
        std::size_t pos = keywordEnd;
        if (!memory_pou_parse_uint16(statement, pos, tick)) {
            return MEMOP_ERR_INIT;
        }
        return enqueueTickBarrier(tick);
    }
    if (memory_pou_starts_with_keyword(statement, "delay", keywordEnd)) {
        uint16_t delayTicks = 0;
        std::size_t pos = keywordEnd;
        if (!memory_pou_parse_uint16(statement, pos, delayTicks)) {
            return MEMOP_ERR_INIT;
        }
        return enqueueDelayBarrier(delayTicks);
    }
    if (memory_pou_starts_with_keyword(statement, "out", keywordEnd)) {
        const std::string symbol = memory_pou_trim(statement.substr(keywordEnd));
        if (symbol.empty()) {
            return MEMOP_ERR_INIT;
        }
        const SymbolInfo* sym = memoryOperator_.findSymbol(symbol);
        if (!sym) {
            return MEMOP_ERR_SYMBOL;
        }
        if (sym->size == 0 || sym->size > 255) {
            return MEMOP_ERR_ASSIGN;
        }
        return enqueueOut(*sym);
    }
    if (memory_pou_starts_with_keyword(statement, "expect", keywordEnd)) {
        const std::string body = memory_pou_trim(statement.substr(keywordEnd));
        std::string lhs;
        std::string rhs;
        uint16_t cmp = kMemoryPouCmpEq;
        if (!memory_pou_parse_expect_body(body, lhs, cmp, rhs)) {
            return MEMOP_ERR_ASSIGN;
        }
        const SymbolInfo* sym = memoryOperator_.findSymbol(lhs);
        if (!sym) {
            return MEMOP_ERR_SYMBOL;
        }
        if (cmp >= kMemoryPouCmpLt && !memoryOperator_.symbolAllowsOrderedCompare(*sym)) {
            return MEMOP_ERR_ASSIGN;
        }
        const std::string assignStmt = lhs + " = " + rhs;
        std::vector<AssignOp> ops;
        const int rc = memoryOperator_.compile(assignStmt, ops);
        if (rc != MEMOP_OK) {
            return rc;
        }
        if (ops.size() != 1 || ops[0].kind != AssignOp::WriteValue) {
            return MEMOP_ERR_ASSIGN;
        }
        parseCmp_ = static_cast<uint8_t>(cmp);
        return enqueueExpect(ops[0]);
    }
    std::vector<AssignOp> ops;
    const int rc = memoryOperator_.compile(statement, ops);
    if (rc != MEMOP_OK) {
        return rc;
    }
    return enqueueFromAssignOps(ops);
}

int MemoryPouEngine::enqueueTickBarrier(uint16_t tick) {
    MemoryPouCmd barrier = memory_pou_make_cmd(static_cast<uint8_t>(kMemoryPouCmdTick));
    barrier.tick = tick;
    cmdQueue_.push_back(barrier);
    return MEMOP_OK;
}

int MemoryPouEngine::enqueueDelayBarrier(uint16_t delayTicks) {
    MemoryPouCmd barrier = memory_pou_make_cmd(static_cast<uint8_t>(kMemoryPouCmdDelay));
    barrier.tick = delayTicks;
    cmdQueue_.push_back(barrier);
    return MEMOP_OK;
}

int MemoryPouEngine::enqueueOut(const SymbolInfo& sym) {
    MemoryPouCmd cmd = memory_pou_make_cmd(static_cast<uint8_t>(kMemoryPouCmdOut));
    cmd.len = static_cast<uint8_t>(sym.size);
    cmd.dst = static_cast<uint32_t>(sym.offset);
    cmd.line = parseLine_;
    cmd.stmtOff = parseStmtOff_;
    cmdQueue_.push_back(cmd);
    return MEMOP_OK;
}

int MemoryPouEngine::enqueueExpect(const AssignOp& op) {
    if (op.value.empty() || op.value.size() > 8 || op.value.size() != op.len) {
        return MEMOP_ERR_ASSIGN;
    }
    MemoryPouCmd cmd = memory_pou_make_cmd(static_cast<uint8_t>(kMemoryPouCmdExpect));
    cmd.len = static_cast<uint8_t>(op.value.size());
    cmd.dst = static_cast<uint32_t>(op.dstOffset);
    cmd.cmp = parseCmp_;
    std::memcpy(&cmd.data, &op.value[0], op.value.size());
    cmd.line = parseLine_;
    cmd.stmtOff = parseStmtOff_;
    cmdQueue_.push_back(cmd);
    return MEMOP_OK;
}

int MemoryPouEngine::enqueueFromAssignOps(const std::vector<AssignOp>& ops) {
    for (std::size_t i = 0; i < ops.size(); ++i) {
        const AssignOp& src = ops[i];
        MemoryPouCmd cmd = memory_pou_make_cmd(0);
        cmd.dst = static_cast<uint32_t>(src.dstOffset);
        if (src.kind == AssignOp::CopyBytes) {
            if (src.len == 0 || src.len > 255) {
                return MEMOP_ERR_ASSIGN;
            }
            cmd.op = static_cast<uint8_t>(kMemoryPouCmdCopy);
            cmd.len = static_cast<uint8_t>(src.len);
            cmd.data = static_cast<uint64_t>(src.srcOffset);
        } else {
            if (src.value.empty() || src.value.size() > 8 || src.value.size() != src.len) {
                return MEMOP_ERR_ASSIGN;
            }
            cmd.op = static_cast<uint8_t>(kMemoryPouCmdAssign);
            cmd.len = static_cast<uint8_t>(src.value.size());
            std::memcpy(&cmd.data, &src.value[0], src.value.size());
        }
        cmdQueue_.push_back(cmd);
    }
    return MEMOP_OK;
}

}  // namespace zmc
