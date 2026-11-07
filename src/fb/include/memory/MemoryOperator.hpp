/* Memory image geometry: type catalog, schema configure, symbol assign. */
#ifndef ZMC_MEMORY_OPERATOR_HPP
#define ZMC_MEMORY_OPERATOR_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "common/thirdlib/nlohmann/json.hpp"

// Error codes returned by MemoryOperator.
enum MemOpCode {
    MEMOP_OK = 0,
    MEMOP_ERR_IO = 1,
    MEMOP_ERR_JSON = 2,
    MEMOP_ERR_LAYOUT = 3,
    MEMOP_ERR_INIT = 4,
    MEMOP_ERR_SYMBOL = 5,
    MEMOP_ERR_ASSIGN = 6,
    MEMOP_ERR_RANGE = 7,
    MEMOP_ERR_EXPECT = 8
};

struct SymbolInfo {
    std::string type;
    std::size_t offset;
    std::size_t size;
};

struct AssignOp {
    enum Kind {
        WriteValue,
        CopyBytes
    };

    Kind kind;
    std::size_t dstOffset;
    std::size_t len;
    std::size_t srcOffset;
    std::vector<unsigned char> value;
};

/* Root slot produced by configure (Image Schema -> offsets). */
struct MemoryRootDecl {
    std::string type;
    std::string name;
    std::size_t offset;
    std::size_t size;
};

// Compiles symbolic assignments into operations on a bound memory block.
class MemoryOperator {
   public:
    MemoryOperator();

    int loadTypeLayoutFile(const std::string& path);
    int loadTypeLayoutContent(const std::string& content);
    int configure(const std::string& initConfig);
    int compile(const std::string& script, std::vector<AssignOp>& ops) const;
    int execute(const std::string& script);

    void bindMemory(void* base, std::size_t bytes);
    int assign(std::size_t offset, const void* value, std::size_t len);
    int copy(std::size_t offset, std::size_t srcOffset, std::size_t len);
    int read(std::size_t offset, void* dst, std::size_t len) const;
    int compare(std::size_t offset, const void* value, std::size_t len) const;
    int compare(std::size_t offset, const void* value, std::size_t len, int cmp) const;
    bool symbolAllowsOrderedCompare(const SymbolInfo& sym) const;

    const SymbolInfo* findSymbol(const std::string& name) const;
    std::size_t rootSize() const { return rootSize_; }
    std::size_t maxAlign() const { return maxAlign_; }
    std::size_t declCount() const { return decls_.size(); }
    const MemoryRootDecl* declAt(std::size_t index) const;

   private:
    struct FieldInfo {
        std::string type;
        std::size_t offset;
        std::size_t size;
    };

    struct TypeInfo {
        std::size_t size;
        std::size_t align;
        std::string kind;
        std::vector<std::pair<std::string, FieldInfo> > fields;
    };

    struct Decl {
        std::string type;
        std::string name;
    };

    int loadTypeLayoutJson(const nlohmann::json& root);
    int parseInitConfig(const std::string& text, std::vector<Decl>& decls) const;
    int encodeLiteral(const std::string& text, const SymbolInfo& dst,
                      std::vector<unsigned char>& bytes) const;
    void addSymbolTree(const std::string& name, const std::string& type,
                       std::size_t baseOffset);
    const TypeInfo* findType(const std::string& type) const;
    const SymbolInfo* findSymbolByOffset(std::size_t offset, std::size_t len) const;

    std::vector<std::pair<std::string, TypeInfo> > types_;
    std::vector<std::pair<std::string, SymbolInfo> > symbols_;
    std::vector<MemoryRootDecl> decls_;
    void* base_;
    std::size_t bytes_;
    std::size_t rootSize_;
    std::size_t maxAlign_;
};

#endif
