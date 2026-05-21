#include "symbol_table.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <utility>

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
        case SymbolKind::Reserved:
            return 8;
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
    if (normalizedType == "enumerated") {
        return 9;
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
    std::string::size_type maxLength = 10;
    for (const TabEntry& entry : tab) {
        if (entry.identifier.length() > maxLength) {
            maxLength = entry.identifier.length(); 
        }
    }
    output << "=== TAB ===" << std::endl;
    output << "identifier" + std::string(maxLength - 10, ' ') + "\t | link\t | obj\t | type\t | ref\t | nrm\t | lev\t | adr" << std::endl;
    for (const TabEntry& entry : tab) {
        output << entry.identifier + std::string(maxLength - entry.identifier.length(), ' ') << "\t | "
               << entry.link << "\t | "
               << entry.obj << "\t | "
               << entry.type << "\t | "
               << entry.ref << "\t | "
               << entry.nrm << "\t | "
               << entry.lev << "\t | "
               << entry.adr << std::endl;
    }

    output << std::endl;
    output << "=== ATAB ===" << std::endl;
    output << "indexType\t | elementType\t | elementRef\t | low\t | high\t | elementSize\t | totalSize" << std::endl;
    for (const ATabEntry& entry : atab) {
        output << entry.indexType << "\t\t\t | "
               << entry.elementType << "\t\t\t | "
               << entry.elementRef << "\t\t\t | "
               << entry.low << "\t | "
               << entry.high << "\t | "
               << entry.elementSize << "\t\t\t | "
               << entry.totalSize << std::endl;
    }

    output << std::endl;
    output << "=== BTAB ===" << std::endl;
    output << "last | lpar\t | psze\t | vsze" << std::endl;
    for (const BTabEntry& entry : btab) {
        output << entry.last << "\t | "
               << entry.lpar << "\t | "
               << entry.psze << "\t | "
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
    const std::vector<std::pair<std::string, SymbolKind>> reservedWords = {
        {"and", SymbolKind::Reserved},
        {"array", SymbolKind::Reserved},
        {"begin", SymbolKind::Reserved},
        {"case", SymbolKind::Reserved},
        {"const", SymbolKind::Reserved},
        {"div", SymbolKind::Reserved},
        {"downto", SymbolKind::Reserved},
        {"do", SymbolKind::Reserved},
        {"else", SymbolKind::Reserved},
        {"end", SymbolKind::Reserved},
        {"for", SymbolKind::Reserved},
        {"function", SymbolKind::Reserved},
        {"if", SymbolKind::Reserved},
        {"mod", SymbolKind::Reserved},
        {"not", SymbolKind::Reserved},
        {"of", SymbolKind::Reserved},
        {"or", SymbolKind::Reserved},
        {"procedure", SymbolKind::Reserved},
        {"program", SymbolKind::Reserved},
        {"record", SymbolKind::Reserved},
        {"repeat", SymbolKind::Reserved},
        {"integer", SymbolKind::Type},
        {"real", SymbolKind::Type},
        {"boolean", SymbolKind::Type},
        {"char", SymbolKind::Type},
        {"string", SymbolKind::Type},
        {"then", SymbolKind::Reserved},
        {"to", SymbolKind::Reserved},
        {"type", SymbolKind::Reserved},
        {"until", SymbolKind::Reserved},
        {"var", SymbolKind::Reserved},
        {"while", SymbolKind::Reserved}
    };

    for (const auto& reservedWord : reservedWords) {
        const std::string& name = reservedWord.first;
        SymbolKind kind = reservedWord.second;
        std::string typeName = kind == SymbolKind::Type ? name : "reserved";
        declareSymbol({name, kind, typeName, currentLevel()});
    }

    declareSymbol({"true", SymbolKind::Constant, "boolean", currentLevel()});
    declareSymbol({"false", SymbolKind::Constant, "boolean", currentLevel()});
    declareSymbol({"read", SymbolKind::Procedure, "procedure", currentLevel()});
    declareSymbol({"readln", SymbolKind::Procedure, "procedure", currentLevel()});
    declareSymbol({"write", SymbolKind::Procedure, "procedure", currentLevel()});
    declareSymbol({"writeln", SymbolKind::Procedure, "procedure", currentLevel()});
}
