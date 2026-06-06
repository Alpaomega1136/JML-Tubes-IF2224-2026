#include "codegen.hpp"
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <cmath>

std::unordered_map<int, std::string> CodeGenerator::hashedStrings;

uint64_t wrapRuntimeType(RuntimeType type, int value) {
    u_int64_t wrapped = static_cast<uint64_t>(value & 0xFFFFFFFF) | (static_cast<uint64_t>(type) << 32);
    return wrapped;
}

pair<RuntimeType, int> unwrapRuntimeType(uint64_t value) {
    RuntimeType type = static_cast<RuntimeType>(value >> 32);
    int val = value & 0xFFFFFFFF;
    return {type, val};
}

string runtimeTypeToString(RuntimeType type) {
    switch(type) {
        case RuntimeType::NONE : return "NONE";
        case RuntimeType::INT : return "INT";
        case RuntimeType::REAL : return "REAL";
        case RuntimeType::CHAR : return "CHAR";
        case RuntimeType::STRING : return "STRING";
        case RuntimeType::ERROR : return "ERROR";
        default: return "";
    }
}

CodeGenerator::CodeGenerator(SymbolTable* table)
    : nextAddress(3),  // 0=static link, 1=dynamic link, 2=return address
      labelCounter(0),
      table(table)
{}

int CodeGenerator::emit(Opcode op, int level, uint64_t operand) {
    int line = static_cast<int>(instructions.size());
    instructions.push_back({line, op, level, operand});
    return line;
}

void CodeGenerator::patch(int instrIndex, int newOperand) {
    if (instrIndex >= 0 && instrIndex < static_cast<int>(instructions.size()))
        instructions[instrIndex].operand = newOperand;
}

int CodeGenerator::nextLine() const {
    return static_cast<int>(instructions.size());
}

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

OprCode CodeGenerator::binopToOpr(const std::string& op) const {
    std::string o = lower(op);
    if (o == "plus"  || o == "+")   return OprCode::ADD;
    if (o == "minus" || o == "-")   return OprCode::SUB;
    if (o == "times" || o == "*")   return OprCode::MUL;
    if (o == "rdiv"  || o == "/")   return OprCode::DIV;
    if (o == "idiv"  || o == "div") return OprCode::DIV;
    if (o == "imod"  || o == "mod") return OprCode::MOD;
    if (o == "eql"   || o == "==")  return OprCode::EQL;
    if (o == "neq"   || o == "<>")  return OprCode::NEQ;
    if (o == "lss"   || o == "<")   return OprCode::LSS;
    if (o == "leq"   || o == "<=")  return OprCode::LEQ;
    if (o == "gtr"   || o == ">")   return OprCode::GTR;
    if (o == "geq"   || o == ">=")  return OprCode::GEQ;
    if (o == "andsy" || o == "and") return OprCode::MUL;
    if (o == "orsy"  || o == "or")  return OprCode::ADD;
    throw std::runtime_error("Unknown binary operator: " + op);
}

int CodeGenerator::resolveAddress(const std::string& name) {
    std::string key = lower(name);
    auto it = varAddress.find(key);
    if (it != varAddress.end()) return it->second;
    if (table->getTab().at(table->lookupTabIndex(name)).type == 6) return allocateArrayAddress(name);
    if (table->getTab().at(table->lookupTabIndex(name)).type == 7) return allocateRecordAddress(name);
    int addr = nextAddress++;
    varAddress[key] = addr;
    return addr;
}

int CodeGenerator::allocateArrayAddress(const std::string& name) {
    std::string key = lower(name);
    
    int addr = nextAddress++;
    int size = table->getATab().at(table->lookup(name)->ref).totalSize;
    nextAddress += size - 1;
    varAddress[key] = addr;
    return addr;
}

int CodeGenerator::allocateRecordAddress(const std::string& name) {
    std::string key = lower(name);

    int addr = nextAddress++;
    int size = typeSize[recordRecord[key]];
    nextAddress += size - 1;
    varAddress[key] = addr;
    return addr;
}

int CodeGenerator::resolveTypeSize(const std::string& name, TypeNode* type) {
    std::string key = lower(name);
    auto it = typeSize.find(key);
    if (it != typeSize.end()) return it->second;
    int size = 0;
    if (table->mapTypeNameToCode(name) <= 5 && table->mapTypeNameToCode(name) > 0) {
        size = 1;
    } else if (table->mapTypeNameToCode(name) == 6) {
        size = table->getATab().at(table->lookup(name)->ref).totalSize;
    } else if (RecordTypeNode* record = dynamic_cast<RecordTypeNode*>(type)) {
        for (FieldPartNode* field : record->fieldList) {
            size += resolveTypeSize(field->fieldType->typeIdent, field->fieldType);
        }
    }
    typeSize[key] = size;
    cout<<key<<" : "<<size<<endl;
    return size;
} 


int CodeGenerator::countVars(ProcCallNode* decls) {
    if (!decls) return 0;
    int count = 0;
    for (ASTNode* child : decls->getChildren()) {
        if (VarDeclNode* decl = dynamic_cast<VarDeclNode*>(child)) {
            count += resolveTypeSize(decl->type->typeIdent, decl->type);
        };
    }
    return count;
}

void CodeGenerator::pushScope() {
    // Tidak perlu melakukan apa-apa; varAddress bisa dibersihkan setelah generate subprogram
}

void CodeGenerator::popScope(const std::unordered_map<std::string, int>& saved,
                              int savedNext) {
    varAddress = saved;
    nextAddress = savedNext;
}

std::string CodeGenerator::opcodeToString(Opcode op) {
    switch (op) {
        case Opcode::LIT: return "LIT";
        case Opcode::LOD: return "LOD";
        case Opcode::STO: return "STO";
        case Opcode::PLO: return "PLO";
        case Opcode::PST: return "PST";
        case Opcode::CAL: return "CAL";
        case Opcode::INT: return "INT";
        case Opcode::JMP: return "JMP";
        case Opcode::JPC: return "JPC";
        case Opcode::OPR: return "OPR";
        case Opcode::RET: return "RET";
    }
    return "???";
}

void CodeGenerator::generate(ASTNode* root) {
    if (!root) return;

    ProgramNode* program = dynamic_cast<ProgramNode*>(root);
    if (!program) {
        std::cerr << "[CodeGen] Root bukan ProgramNode\n";
        return;
    }

    auto ch = program->getChildren();
    if (ch.size() < 2) {
        std::cerr << "[CodeGen] ProgramNode tidak memiliki cukup children\n";
        return;
    }

    ProcCallNode* decls = dynamic_cast<ProcCallNode*>(ch[0]);
    ProcCallNode* block = dynamic_cast<ProcCallNode*>(ch[1]);

    if (decls) {
        for (ASTNode* child : decls->getChildren()) {
            if (TypeDeclNode* var = dynamic_cast<TypeDeclNode*>(child)) {
                resolveTypeSize(var->name, var->simpleType);
                if (RecordTypeNode* rec = dynamic_cast<RecordTypeNode*>(var->simpleType)) {
                    int offset = 0;
                    for (FieldPartNode* field : rec->fieldList) {
                        fieldOffset[lower(var->name)][lower(field->fieldIdent)] = offset;
                        offset += resolveTypeSize(field->fieldType->typeIdent, field->fieldType);
                    }
                }
            }
        }
    }

    int varCount = decls ? countVars(decls) : 0;
    int memSize  = 3 + varCount;

    if (decls) {
        for (ASTNode* child : decls->getChildren()) {
            if (VarDeclNode* var = dynamic_cast<VarDeclNode*>(child)) {
                resolveAddress(var->name);
                if (table->getTab().at(table->lookupTabIndex(var->name)).type == 7) {
                    recordRecord[lower(var->name)] = lower(var->type->typeIdent);
                }
            }
        }
    }

    emit(Opcode::INT, 0, memSize);

    if (decls) genDeclarations(decls);

    if (block) genBlock(block);

    emit(Opcode::RET, 0, 0);
}

void CodeGenerator::genDeclarations(ProcCallNode* decls) {
    if (!decls) return;
    for (ASTNode* child : decls->getChildren()) {
        if (ProcDeclNode* proc = dynamic_cast<ProcDeclNode*>(child)) {
            genProcDecl(proc);
        } else if (FuncDeclNode* func = dynamic_cast<FuncDeclNode*>(child)) {
            genFuncDecl(func);
        }
    }
}

void CodeGenerator::genBlock(ProcCallNode* block) {
    if (!block) return;
    for (ASTNode* child : block->getChildren()) {
        genStatement(child);
    }
}

void CodeGenerator::genProcDecl(ProcDeclNode* node) {
    if (!node) return;

    int jmpOver = emit(Opcode::JMP, 0, 0);

    int procStart = nextLine();
    subprogramAddress[lower(node->name)] = procStart;

    auto savedVars = varAddress;
    int savedNext  = nextAddress;
    varAddress.clear();
    nextAddress = 3;

    for (ParameterNode* param : node->parameterList) {
        resolveAddress(param->name);
    }

    auto ch = node->getChildren();
    ProcCallNode* innerDecls = ch.size() > 0 ? dynamic_cast<ProcCallNode*>(ch[0]) : nullptr;
    ProcCallNode* innerBlock = ch.size() > 1 ? dynamic_cast<ProcCallNode*>(ch[1]) : nullptr;

    int localVars = innerDecls ? countVars(innerDecls) : 0;
    if (innerDecls) {
        for (ASTNode* c : innerDecls->getChildren()) {
            if (VarDeclNode* v = dynamic_cast<VarDeclNode*>(c)) {
                resolveAddress(v->name);
            }
        }
    }

    int memSize = 3 + static_cast<int>(node->parameterList.size()) + localVars;
    emit(Opcode::INT, 0, memSize);

    if (innerDecls) genDeclarations(innerDecls);
    if (innerBlock) genBlock(innerBlock);

    emit(Opcode::RET, 0, 0);

    patch(jmpOver, nextLine());

    popScope(savedVars, savedNext);
}

void CodeGenerator::genFuncDecl(FuncDeclNode* node) {
    if (!node) return;

    int jmpOver = emit(Opcode::JMP, 0, 0);

    int funcStart = nextLine();
    subprogramAddress[lower(node->name)] = funcStart;

    auto savedVars = varAddress;
    int savedNext  = nextAddress;
    varAddress.clear();
    nextAddress = 3;

    for (ParameterNode* param : node->parameterList) {
        resolveAddress(param->name);
    }

    auto ch = node->getChildren();
    ProcCallNode* innerDecls = ch.size() > 0 ? dynamic_cast<ProcCallNode*>(ch[0]) : nullptr;
    ProcCallNode* innerBlock = ch.size() > 1 ? dynamic_cast<ProcCallNode*>(ch[1]) : nullptr;

    int localVars = innerDecls ? countVars(innerDecls) : 0;
    if (innerDecls) {
        for (ASTNode* c : innerDecls->getChildren()) {
            if (VarDeclNode* v = dynamic_cast<VarDeclNode*>(c)) {
                resolveAddress(v->name);
            }
        }
    }

    resolveAddress(node->name);

    int memSize = 3 + static_cast<int>(node->parameterList.size()) + localVars + 1;
    emit(Opcode::INT, 0, memSize);

    if (innerDecls) genDeclarations(innerDecls);
    if (innerBlock) genBlock(innerBlock);

    int retAddr = resolveAddress(node->name);
    emit(Opcode::LOD, 0, retAddr);

    emit(Opcode::RET, 0, 0);

    patch(jmpOver, nextLine());
    popScope(savedVars, savedNext);
}

void CodeGenerator::genStatement(ASTNode* node) {
    if (!node) return;

    if (AssignNode*   n = dynamic_cast<AssignNode*>(node))   { genAssign(n);   return; }
    if (IfNode*       n = dynamic_cast<IfNode*>(node))       { genIf(n);       return; }
    if (WhileNode*    n = dynamic_cast<WhileNode*>(node))    { genWhile(n);    return; }
    if (RepeatNode*   n = dynamic_cast<RepeatNode*>(node))   { genRepeat(n);   return; }
    if (ForNode*      n = dynamic_cast<ForNode*>(node))      { genFor(n);      return; }
    if (CaseNode*     n = dynamic_cast<CaseNode*>(node))     { genCase(n);     return; }
    if (ProcCallNode* n = dynamic_cast<ProcCallNode*>(node)) {
        if (n->name == "__block__")          { genBlock(n);        return; }
        if (n->name == "__declarations__")   { genDeclarations(n); return; }
        genProcCall(n);
        return;
    }
}

void CodeGenerator::genAssign(AssignNode* node) {
    if (!node) return;

    genExpression(node->value);

    VarNode* targetVar = dynamic_cast<VarNode*>(node->target);
    if (targetVar) {
        int addr = resolveAddress(targetVar->name);
        emit(Opcode::STO, 0, addr);
    } else if (ArrayAccessNode* targetVar = dynamic_cast<ArrayAccessNode*>(node->target)) {
        int addr = resolveAddress(targetVar->name);
        genExpression(targetVar->idx);

        int inset = table->getATab().at(table->lookup(targetVar->name)->ref).low;

        emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, inset));

        emit(Opcode::OPR, 0, static_cast<int>(OprCode::SUB));

        emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, addr));

        emit(Opcode::OPR, 0, static_cast<int>(OprCode::ADD));

        emit(Opcode::PST, 0, 0);
    } else if (RecordAccessNode* targetVar = dynamic_cast<RecordAccessNode*>(node->target)) {
        int addr = resolveAddress(targetVar->name);
        
        emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, fieldOffset[recordRecord[lower(targetVar->name)]][targetVar->fieldName]));

        emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, addr));

        emit(Opcode::OPR, 0, static_cast<int>(OprCode::ADD));

        emit(Opcode::PST, 0, 0);
    }
}

void CodeGenerator::genIf(IfNode* node) {
    if (!node) return;

    genExpression(node->condition);

    // JPC ke else (atau end jika tidak ada else)
    int jpcIdx = emit(Opcode::JPC, 0, 0);

    genStatement(node->then);

    if (node->elseThen) {
        int jmpIdx = emit(Opcode::JMP, 0, 0);
        patch(jpcIdx, nextLine()); // JPC melompat ke sini (else)
        genStatement(node->elseThen);
        patch(jmpIdx, nextLine()); // JMP melompat ke sini (end)
    } else {
        patch(jpcIdx, nextLine()); // JPC melompat ke sini (end)
    }
}

void CodeGenerator::genWhile(WhileNode* node) {
    if (!node) return;

    int loopStart = nextLine();

    genExpression(node->condition);

    int jpcIdx = emit(Opcode::JPC, 0, 0);

    genStatement(node->statement);

    emit(Opcode::JMP, 0, loopStart);

    patch(jpcIdx, nextLine());
}

void CodeGenerator::genRepeat(RepeatNode* node) {
    if (!node) return;

    int loopStart = nextLine();

    genStatement(node->statement);

    genExpression(node->untilCondition);

    int jpcIdx = emit(Opcode::JPC, 0, 0);
    emit(Opcode::JMP, 0, loopStart);
    patch(jpcIdx, nextLine());
}

void CodeGenerator::genFor(ForNode* node) {
    if (!node) return;

    genAssign(node->traversalAssign);

    VarNode* counterVar = dynamic_cast<VarNode*>(node->traversalAssign->target);
    if (!counterVar) return;
    int counterAddr = resolveAddress(counterVar->name);

    int loopStart = nextLine();

    // TODO: kalau ada downto, tambahkan fieldnya di ForNode
    emit(Opcode::LOD, 0, counterAddr);
    genExpression(node->to);
    emit(Opcode::OPR, 0, static_cast<int>(OprCode::LEQ));

    int jpcIdx = emit(Opcode::JPC, 0, 0);

    genStatement(node->statement);

    emit(Opcode::LOD, 0, counterAddr);
    emit(Opcode::LIT, 0, 1);
    emit(Opcode::OPR, 0, static_cast<int>(OprCode::ADD));
    emit(Opcode::STO, 0, counterAddr);

    emit(Opcode::JMP, 0, loopStart);

    patch(jpcIdx, nextLine());
}

void CodeGenerator::genCase(CaseNode* node) {
    if (!node) return;

    std::vector<int> jmpEndList;

    for (CaseBlockNode* caseBlock : node->caseBlocks) {
        genExpression(node->condition);
        genExpression(caseBlock->caseCondition);
        emit(Opcode::OPR, 0, static_cast<int>(OprCode::EQL));
        int jpcIdx = emit(Opcode::JPC, 0, 0);

        genStatement(caseBlock->statement);

        int jmpEnd = emit(Opcode::JMP, 0, 0);
        jmpEndList.push_back(jmpEnd);

        patch(jpcIdx, nextLine());
    }

    int endLine = nextLine();
    for (int idx : jmpEndList) {
        patch(idx, endLine);
    }
}

void CodeGenerator::genProcCall(ProcCallNode* node) {
    if (!node) return;

    std::string name = lower(node->name);

    if (name == "writeln") {
        for (ValueNode* arg : node->args) {
            genExpression(arg);
            emit(Opcode::OPR, 0, static_cast<int>(OprCode::WRTLN));
        }
        return;
    }

    if (name == "write") {
        for (ValueNode* arg : node->args) {
            genExpression(arg);
            emit(Opcode::OPR, 0, static_cast<int>(OprCode::WRT));
        }
        return;
    }

    auto it = subprogramAddress.find(name);
    if (it != subprogramAddress.end()) {
        for (ValueNode* arg : node->args) {
            genExpression(arg);
        }
        emit(Opcode::CAL, 0, it->second);
    } else {
        std::cerr << "[CodeGen] Warning: prosedur '" << node->name << "' tidak ditemukan\n";
    }
}

void CodeGenerator::genExpression(ValueNode* node) {
    if (!node) return;

    if (NumberNode*       n = dynamic_cast<NumberNode*>(node))      { genNumber(n);   return; }
    if (CharNode*         n = dynamic_cast<CharNode*>(node))        { genChar(n);     return; }
    if (StringNode*       n = dynamic_cast<StringNode*>(node))      { genString(n);   return; }
    if (VarNode*          n = dynamic_cast<VarNode*>(node))         { genVar(n);      return; }
    if (ArrayAccessNode*  n = dynamic_cast<ArrayAccessNode*>(node)) { genArrayAccess(n);      return; }
    if (RecordAccessNode* n = dynamic_cast<RecordAccessNode*>(node)){ genRecordAccess(n);      return; }
    if (BinOpNode*        n = dynamic_cast<BinOpNode*>(node))       { genBinOp(n);    return; }
    if (UnaryOpNode*      n = dynamic_cast<UnaryOpNode*>(node))     { genUnaryOp(n);  return; }
    if (FuncCallNode*     n = dynamic_cast<FuncCallNode*>(node))    { genFuncCall(n); return; }

    std::cerr << "[CodeGen] Warning: tipe ekspresi tidak dikenal\n";
}

void CodeGenerator::genNumber(NumberNode* node) {
    try {
        
        if (node->num.find('.') == std::string::npos) {
            int val = std::stoi(node->num);
            emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, val));
        } else {
            double val = std::stod(node->num);
            float f = static_cast<float>(val);
            uint64_t bits;
            std::memcpy(&bits, &f, sizeof(f));
            emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::REAL, static_cast<int>(bits)));
        }
    } catch (...) {
        emit(Opcode::LIT, 0, 0);
    }
}

void CodeGenerator::genChar(CharNode* node) {
    emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::CHAR, static_cast<int>(node->c)));
}

void CodeGenerator::genString(StringNode* node) {
    // TODO: Encode string sebagai literal khusus
    // Interpreter perlu tabel string terpisah
    // Emit LIT dengan nilai hash sederhana sama interpreter
    int hashValue = static_cast<int>(std::hash<std::string>{}(node->str) & 0x7FFFFFFF);
    emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::STRING, hashValue));
    hashedStrings[hashValue] = node->str;
}

void CodeGenerator::genVar(VarNode* node) {
    int addr = resolveAddress(node->name);  
    emit(Opcode::LOD, 0, addr);
}

void CodeGenerator::genArrayAccess(ArrayAccessNode* node) {
    int addr = resolveAddress(node->name);

    genExpression(node->idx);

    int inset = table->getATab().at(table->lookup(node->name)->ref).low;

    emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, inset));

    emit(Opcode::OPR, 0, static_cast<int>(OprCode::SUB));

    emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, addr));

    emit(Opcode::OPR, 0, static_cast<int>(OprCode::ADD));

    emit(Opcode::PLO, 0, 0);
}

void CodeGenerator::genRecordAccess(RecordAccessNode* node) {
    int addr = resolveAddress(node->name);
        
    emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, fieldOffset[recordRecord[lower(node->name)]][lower(node->fieldName)]));

    emit(Opcode::LIT, 0, wrapRuntimeType(RuntimeType::INT, addr));

    emit(Opcode::OPR, 0, static_cast<int>(OprCode::ADD));

    emit(Opcode::PLO, 0, 0);
}

void CodeGenerator::genBinOp(BinOpNode* node) {
    genExpression(node->left);
    genExpression(node->right);
    OprCode opr = binopToOpr(node->op);
    emit(Opcode::OPR, 0, static_cast<int>(opr));
}

void CodeGenerator::genUnaryOp(UnaryOpNode* node) {
    genExpression(node->value);
    std::string op = lower(node->op);
    if (op == "minus" || op == "-") {
        emit(Opcode::OPR, 0, static_cast<int>(OprCode::NEG));
    } else if (op == "not") {
        emit(Opcode::LIT, 0, 1);
        emit(Opcode::OPR, 0, static_cast<int>(OprCode::EQL));
    }
}

void CodeGenerator::genFuncCall(FuncCallNode* node) {
    std::string name = lower(node->name);

    auto it = subprogramAddress.find(name);
    if (it != subprogramAddress.end()) {
        for (ValueNode* arg : node->args) {
            genExpression(arg);
        }
        emit(Opcode::CAL, 0, it->second);
    } else {
        std::cerr << "[CodeGen] Warning: fungsi '" << node->name << "' tidak ditemukan\n";
        emit(Opcode::LIT, 0, 0);
    }
}

void CodeGenerator::print(std::ostream& out) const {
    for (const Instruction& instr : instructions) {
        out << instr.line << " "
            << opcodeToString(instr.opcode) << " "
            << instr.level << " "
            << unwrapRuntimeType(instr.operand).second << "\n";
    }
}

const std::vector<Instruction>& CodeGenerator::getInstructions() const {
    return instructions;
}