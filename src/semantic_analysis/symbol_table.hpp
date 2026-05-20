#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

enum class SymbolKind {
    Variable,
    Constant,
    Type,
    Function,
    Procedure,
    Parameter
};

struct SymbolEntry {
    std::string name;
    SymbolKind kind;
    std::string typeName;
    int lexicalLevel;
    std::vector<std::string> parameterTypes;
    std::vector<std::string> parameterNames;
};

struct TabEntry {
    std::string identifier;
    int link;
    int obj;
    int type;
    int ref;
    int nrm;
    int lev;
    int adr;
};

struct ATabEntry {
    int indexType;
    int elementType;
    int low;
    int high;
    int elementSize;
    int totalSize;
};

struct BTabEntry {
    int last;
    int lpar;
    int psze;
    int vsze;
};

class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, SymbolEntry>> scopes;
    std::vector<TabEntry> tab;
    std::vector<ATabEntry> atab;
    std::vector<BTabEntry> btab;

    void addPredefinedSymbols();
    void appendTabEntry(const SymbolEntry& entry);

public:
    SymbolTable();

    void enterScope();
    void exitScope();

    bool declareSymbol(const SymbolEntry& entry);
    SymbolEntry* lookup(const std::string& name);
    SymbolEntry* lookupCurrentScope(const std::string& name);

    int currentLevel() const;
    int mapKindToObj(SymbolKind kind) const;
    int mapTypeNameToCode(const std::string& typeName) const;

    const std::vector<TabEntry>& getTab() const;
    const std::vector<ATabEntry>& getATab() const;
    const std::vector<BTabEntry>& getBTab() const;
    void printSpecTables() const;
    void printSpecTables(std::ostream& output) const;
};
