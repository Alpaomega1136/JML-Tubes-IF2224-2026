#include "symbol_table.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

static std::string toLowerString(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

SymbolTable::SymbolTable() {
    enterScope();
    addPredefinedSymbols();
}

void SymbolTable::enterScope() {
    scopes.emplace_back();
    btab.push_back({0, 0, 0, 0});
    activeBlockStack.push_back(static_cast<int>(btab.size()) - 1);
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
        activeBlockStack.pop_back();
    }
}

bool SymbolTable::declareSymbol(const SymbolEntry& entry) {
    if (scopes.empty()) {
        enterScope();
    }

    auto& currentScope = scopes.back();
    std::string normalizedName = toLowerString(entry.name);
    if (currentScope.find(normalizedName) != currentScope.end()) {
        return false;
    }

    SymbolEntry storedEntry = entry;
    storedEntry.lexicalLevel = currentLevel();
    int activeBlock = activeBlockStack.empty() ? -1 : activeBlockStack.back();
    storedEntry.link = activeBlock < 0 ? 0 : btab[activeBlock].last;
    appendTabEntry(storedEntry);
    storedEntry.tabIndex = static_cast<int>(tab.size()) - 1;
    currentScope[normalizedName] = storedEntry;

    if (activeBlock >= 0) {
        btab[activeBlock].last = storedEntry.tabIndex;
        if (storedEntry.kind == SymbolKind::Parameter) {
            btab[activeBlock].lpar = storedEntry.tabIndex;
            btab[activeBlock].psze += 1;
        } else if (storedEntry.kind == SymbolKind::Variable) {
            btab[activeBlock].vsze += 1;
        }
    }

    return true;
}

SymbolEntry* SymbolTable::lookup(const std::string& name) {
    std::string normalizedName = toLowerString(name);
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(normalizedName);
        if (found != it->end()) {
            return &found->second;
        }
    }

    return nullptr;
}

SymbolEntry* SymbolTable::lookupCurrentScope(const std::string& name) {
    if (scopes.empty()) {
        return nullptr;
    }

    auto& currentScope = scopes.back();
    auto found = currentScope.find(toLowerString(name));
    if (found != currentScope.end()) {
        return &found->second;
    }

    return nullptr;
}

int SymbolTable::lookupTabIndex(const std::string& name) const {
    std::string normalizedName = toLowerString(name);
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(normalizedName);
        if (found != it->end()) {
            return found->second.tabIndex;
        }
    }

    return -1;
}

int SymbolTable::currentLevel() const {
    if (scopes.empty()) {
        return -1;
    }

    return static_cast<int>(scopes.size()) - 1;
}

int SymbolTable::mapKindToObj(SymbolKind kind) const {
    switch (kind) {
        case SymbolKind::Program:
            return 0;
        case SymbolKind::Variable:
            return 1;
        case SymbolKind::Constant:
            return 2;
        case SymbolKind::Type:
            return 3;
        case SymbolKind::Function:
            return 4;
        case SymbolKind::Procedure:
            return 5;
        case SymbolKind::Parameter:
            return 6;
        case SymbolKind::Field:
            return 7;
        default:
            return 0;
    }
}

int SymbolTable::mapTypeNameToCode(const std::string& typeName) const {
    std::string normalizedType = toLowerString(typeName);

    if (normalizedType == "integer") {
        return 1;
    }
    if (normalizedType == "real") {
        return 2;
    }
    if (normalizedType == "char") {
        return 3;
    }
    if (normalizedType == "boolean") {
        return 4;
    }
    if (normalizedType == "string") {
        return 5;
    }
    if (normalizedType == "array") {
        return 6;
    }
    if (normalizedType == "record") {
        return 7;
    }
    if (normalizedType == "procedure") {
        return 8;
    }

    return 0;
}

int SymbolTable::addArrayType(int indexType, int elementType, int elementRef, int low, int high, int elementSize) {
    int totalSize = 0;
    if (high >= low) {
        totalSize = (high - low + 1) * elementSize;
    }

    atab.push_back({
        indexType,
        elementType,
        elementRef,
        low,
        high,
        elementSize,
        totalSize
    });

    return static_cast<int>(atab.size()) - 1;
}

int SymbolTable::addBlockEntry() {
    btab.push_back({0, 0, 0, 0});
    return static_cast<int>(btab.size()) - 1;
}

int SymbolTable::addRecordField(int blockIndex, const std::string& name, const std::string& typeName, int typeCode, int ref, int adr) {
    if (blockIndex < 0 || blockIndex >= static_cast<int>(btab.size())) {
        return -1;
    }

    SymbolEntry entry{
        name,
        SymbolKind::Field,
        typeName,
        currentLevel()
    };
    entry.link = btab[blockIndex].last;
    entry.typeCode = typeCode;
    entry.ref = ref;
    entry.adr = adr;
    appendTabEntry(entry);

    int tabIndex = static_cast<int>(tab.size()) - 1;
    btab[blockIndex].last = tabIndex;
    btab[blockIndex].vsze += 1;
    return tabIndex;
}

const std::vector<TabEntry>& SymbolTable::getTab() const {
    return tab;
}

const std::vector<ATabEntry>& SymbolTable::getATab() const {
    return atab;
}

const std::vector<BTabEntry>& SymbolTable::getBTab() const {
    return btab;
}

void SymbolTable::printSpecTables() const {
    printSpecTables(std::cout);
}

void SymbolTable::printSpecTables(std::ostream& output) const {
    output << "=== TAB ===" << std::endl;
    output << "identifier | link | obj | type | ref | nrm | lev | adr" << std::endl;
    for (const TabEntry& entry : tab) {
        output << entry.identifier << " | "
               << entry.link << " | "
               << entry.obj << " | "
               << entry.type << " | "
               << entry.ref << " | "
               << entry.nrm << " | "
               << entry.lev << " | "
               << entry.adr << std::endl;
    }

    output << std::endl;
    output << "=== ATAB ===" << std::endl;
    output << "indexType | elementType | elementRef | low | high | elementSize | totalSize" << std::endl;
    for (const ATabEntry& entry : atab) {
        output << entry.indexType << " | "
               << entry.elementType << " | "
               << entry.elementRef << " | "
               << entry.low << " | "
               << entry.high << " | "
               << entry.elementSize << " | "
               << entry.totalSize << std::endl;
    }

    output << std::endl;
    output << "=== BTAB ===" << std::endl;
    output << "last | lpar | psze | vsze" << std::endl;
    for (const BTabEntry& entry : btab) {
        output << entry.last << " | "
               << entry.lpar << " | "
               << entry.psze << " | "
               << entry.vsze << std::endl;
    }
}

void SymbolTable::appendTabEntry(const SymbolEntry& entry) {
    tab.push_back({
        entry.name,
        entry.link,
        mapKindToObj(entry.kind),
        entry.typeCode >= 0 ? entry.typeCode : mapTypeNameToCode(entry.typeName),
        entry.ref,
        entry.nrm,
        entry.lexicalLevel,
        entry.adr
    });
}

void SymbolTable::addPredefinedSymbols() {
    declareSymbol({"integer", SymbolKind::Type, "integer", currentLevel()});
    declareSymbol({"real", SymbolKind::Type, "real", currentLevel()});
    declareSymbol({"char", SymbolKind::Type, "char", currentLevel()});
    declareSymbol({"boolean", SymbolKind::Type, "boolean", currentLevel()});
    declareSymbol({"string", SymbolKind::Type, "string", currentLevel()});
    declareSymbol({"true", SymbolKind::Constant, "boolean", currentLevel()});
    declareSymbol({"false", SymbolKind::Constant, "boolean", currentLevel()});
    declareSymbol({"read", SymbolKind::Procedure, "procedure", currentLevel()});
    declareSymbol({"readln", SymbolKind::Procedure, "procedure", currentLevel()});
    declareSymbol({"write", SymbolKind::Procedure, "procedure", currentLevel()});
    declareSymbol({"writeln", SymbolKind::Procedure, "procedure", currentLevel()});
}
