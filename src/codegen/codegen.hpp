#pragma once

#include "../semantic_analysis/ast.hpp"
#include "../semantic_analysis/symbol_table.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <cstring>
#include <bitset>
#include <cstdint>

// Instruksi TAC
// Format: [line] [opcode] [level] [operand]
enum class Opcode {
    LIT,  // Load Literal
    LOD,  // Load Value dari address
    STO,  // Store Value ke address
    PLO,  // Load Value ke address yang di pop dari stack
    PST,  // Store Value ke address yang di pop dari stack
    CAL,  // Call fungsi/prosedur
    INT,  // Initiate Memory
    JMP,  // Unconditional Jump
    JPC,  // Conditional Jump (jump if false)
    OPR,  // Operation
    RET   // Return
};

// Instruksi OPR
enum class OprCode {
    NEG   = 1,   // Negasi
    ADD   = 2,   // Tambah
    SUB   = 3,   // Kurang
    MUL   = 4,   // Kali
    DIV   = 5,   // Bagi
    MOD   = 6,   // Modulus
    EQL   = 7,   // ==
    NEQ   = 8,   // <>
    LSS   = 9,   // <
    GEQ   = 10,  // >=
    GTR   = 11,  // >
    LEQ   = 12,  // <=
    WRT   = 13,  // Write
    WRTLN = 14   // Writeln
};

enum class RuntimeType {
    NONE = 0,
    INT = 1,
    CHAR = 2,
    STRING = 3,
    REAL = 4,
    BOOLEAN = 5,
    ERROR = 6
};

uint64_t wrapRuntimeType(RuntimeType type, int value);

pair<RuntimeType, int> unwrapRuntimeType(uint64_t value);

string runtimeTypeToString(RuntimeType type);

// Satu baris instruksi TAC
struct Instruction {
    int     line;    // nomor baris
    Opcode  opcode;  // jenis instruksi
    int     level;   // lexical level (biasanya 0)
    uint64_t operand; // nilai operand
};

class CodeGenerator {
private:
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, int> typeSize;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> fieldOffset;
    std::unordered_map<std::string, std::string> recordRecord;
    std::unordered_map<std::string, int> varAddress;
    int nextAddress;
    int labelCounter;
    std::unordered_map<std::string, int> subprogramAddress;
    SymbolTable* table;

    int emit(Opcode op, int level, uint64_t operand);
    void patch(int instrIndex, int newOperand);
    int nextLine() const;

    OprCode binopToOpr(const std::string& op) const;

    void genExpression(ValueNode* node);
    void genBinOp(BinOpNode* node);
    void genUnaryOp(UnaryOpNode* node);
    void genNumber(NumberNode* node);
    void genChar(CharNode* node);
    void genString(StringNode* node);
    void genVar(VarNode* node);
    void genArrayAccess(ArrayAccessNode* node);
    void genRecordAccess(RecordAccessNode* node);
    void genFuncCall(FuncCallNode* node);

    void genStatement(ASTNode* node);
    void genAssign(AssignNode* node);
    void genIf(IfNode* node);
    void genWhile(WhileNode* node);
    void genRepeat(RepeatNode* node);
    void genFor(ForNode* node);
    void genCase(CaseNode* node);
    void genProcCall(ProcCallNode* node);

    void genBlock(ProcCallNode* block);
    void genDeclarations(ProcCallNode* decls);
    void genProcDecl(ProcDeclNode* node);
    void genFuncDecl(FuncDeclNode* node);

    int countVars(ProcCallNode* decls);
    int resolveAddress(const std::string& name);
    int resolveTypeSize(const std::string& name, TypeNode* type);
    int allocateArrayAddress(const std::string& name);
    int allocateRecordAddress(const std::string& name);

    void pushScope();
    void popScope(const std::unordered_map<std::string, int>& saved,
                  int savedNext);

public:
    static std::unordered_map<int, std::string> hashedStrings;
    CodeGenerator(SymbolTable* table);
    void generate(ASTNode* root);
    void print(std::ostream& out) const;
    const std::vector<Instruction>& getInstructions() const;
    static std::string opcodeToString(Opcode op);
};