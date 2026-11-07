/* Timed in-place poke engine over a bound memory image (no FB type policy). */
#ifndef ZMC_MEMORY_POU_ENGINE_HPP
#define ZMC_MEMORY_POU_ENGINE_HPP

#include "memory/MemoryOperator.hpp"

#include <cstddef>
#include <stdint.h>
#include <string>
#include <vector>

namespace zmc {

enum MemoryPouCmdOp {
    kMemoryPouCmdNop,
    kMemoryPouCmdAssign,
    kMemoryPouCmdCopy,
    kMemoryPouCmdTick,
    kMemoryPouCmdDelay,
    kMemoryPouCmdOut,
    kMemoryPouCmdExpect
};

enum MemoryPouCaptureKind {
    kMemoryPouCaptureOut = 0,
    kMemoryPouCaptureExpectOk = 1,
    kMemoryPouCaptureExpectFail = 2
};

/* Expect comparison operator; stored in MemoryPouCmd.cmp for kMemoryPouCmdExpect. */
enum MemoryPouCmpOp {
    kMemoryPouCmpEq = 0,
    kMemoryPouCmpNe = 1,
    kMemoryPouCmpLt = 2,
    kMemoryPouCmpLe = 3,
    kMemoryPouCmpGt = 4,
    kMemoryPouCmpGe = 5
};

struct MemoryPouCmd {
    uint8_t op;
    uint8_t len;
    uint8_t cmp;
    uint8_t _pad;
    uint16_t tick;
    uint32_t dst;
    uint64_t data;
    /* -1 when the statement has no source location (assign/delay/tick). */
    int32_t line;
    int32_t stmtOff;
};

/* One captured sample: raw bytes live in engine valueBuf_. */
struct MemoryPouCaptureInfo {
    uint32_t tick;
    int32_t line;
    int32_t stmtOff;
    uint32_t src;
    uint32_t bufOff;
    uint32_t len;
    uint32_t kind;
};

/* Geometry + scheduled assign/copy/delay/out/expect against a caller-owned image. */
class MemoryPouEngine {
   public:
    MemoryPouEngine();

    int errorCode() const { return errorCode_; }
    MemoryOperator& memoryOperator() { return memoryOperator_; }
    const MemoryOperator& memoryOperator() const { return memoryOperator_; }

    int loadTypeLayout(const std::string& content);
    int configure(const std::string& initConfig);
    void bindMemory(void* base, std::size_t bytes);
    int parseScheduledScript(const std::string& script);
    void cmdAdvance();

    std::size_t captureCount() const { return captureInfoVec_.size(); }
    const MemoryPouCaptureInfo* captureAt(std::size_t index) const;
    const unsigned char* valueBufData() const;
    std::size_t valueBufSize() const { return valueBuf_.size(); }

   private:
    void executeCommandsAtCurrentTick();
    int addScheduledStatement(const std::string& statement);
    int enqueueTickBarrier(uint16_t tick);
    int enqueueDelayBarrier(uint16_t delayTicks);
    int enqueueFromAssignOps(const std::vector<AssignOp>& ops);
    int enqueueOut(const SymbolInfo& sym);
    int enqueueExpect(const AssignOp& op);
    int appendCapture(const MemoryPouCmd& cmd, const unsigned char* bytes,
                      MemoryPouCaptureKind kind);
    int applyCmd(const MemoryPouCmd& cmd);

    MemoryOperator memoryOperator_;
    int errorCode_;
    std::vector<MemoryPouCmd> cmdQueue_;
    std::vector<unsigned char> valueBuf_;
    std::vector<MemoryPouCaptureInfo> captureInfoVec_;
    std::size_t cmdIndex_;
    unsigned int tick_;
    unsigned int waitUntilTick_;
    /* Filled by parseScheduledScript for the current statement; -1 if unused. */
    int32_t parseLine_;
    int32_t parseStmtOff_;
    /* Expect compare op for the current statement; 0 if unused. */
    uint8_t parseCmp_;
};

}  // namespace zmc

#endif
