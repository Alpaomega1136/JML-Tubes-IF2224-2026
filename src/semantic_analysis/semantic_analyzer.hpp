#pragma once

#include "ast.hpp"
#include "symbol_table.hpp"
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class SemanticAnalyzer {
private:
    struct TypeInfo {
        std::string kind = "simple";
        std::string baseType = "unknown";
        std::string indexType = "unknown";
        std::string elementType = "unknown";
        bool hasBounds = false;
        int low = 0;
        int high = 0;
        int ref = 0;
        std::unordered_map<std::string, std::string> fields;
        std::vector<std::string> members;
    };

    SymbolTable symbolTable;
    std::vector<std::string> errors;
    std::unordered_map<std::string, TypeInfo> typeRegistry;

    void analyzeProgram(ProgramNode* program);
    void analyzeNode(ASTNode* node);
    void analyzeDeclaration(ASTNode* node);
    void analyzeProcDeclaration(ProcDeclNode* procDecl);
    void analyzeFuncDeclaration(FuncDeclNode* funcDecl);
    void analyzeSubprogramChildren(ASTNode* subprogramNode);
    void declareParameters(const std::vector<ParameterNode*>& parameters);
    std::vector<std::string> collectParameterTypes(const std::vector<ParameterNode*>& parameters) const;
    std::vector<std::string> collectParameterNames(const std::vector<ParameterNode*>& parameters) const;
    void analyzeStatement(ASTNode* node);
    void analyzeExpression(ValueNode* expr);
    void declareOrReport(const SymbolEntry& entry);
    void checkIdentifierDeclared(const std::string& name);
    bool hasError(const std::string& message) const;
    void addError(const std::string& message);
    void initializeBuiltinTypes();
    bool isContainerNode(ProcCallNode* callNode) const;
    bool isVariadicBuiltin(const std::string& name) const;
    bool isNumericType(const std::string& typeName) const;
    bool isSimpleAssignableType(const std::string& typeName) const;
    bool isBooleanType(const std::string& typeName) const;
    bool isAssignmentCompatible(const std::string& targetType, const std::string& valueType) const;
    bool isComparisonCompatible(const std::string& leftType, const std::string& rightType) const;
    bool isValueWithinType(const std::string& targetType, ValueNode* valueNode) const;
    void checkCallArguments(const std::string& callName, const std::vector<ValueNode*>& args, SymbolEntry* entry);
    void registerTypeDeclaration(TypeDeclNode* typeDecl);
    TypeInfo describeType(TypeNode* typeNode);
    bool extractOrdinalValue(ValueNode* valueNode, int& value) const;
    int typeCodeFor(const std::string& typeName) const;
    std::string resolveTypeName(const std::string& typeName) const;
    std::string normalizeName(const std::string& text) const;
    std::string targetDisplayName(ValueNode* value) const;
    std::string inferExpressionType(ValueNode* expr);
    std::string typeNameFromTypeNode(TypeNode* typeNode) const;
    std::string typeNameFromValueNode(ValueNode* valueNode) const;

public:
    void analyze(ASTNode* root);
    void visitProgram(ProgramNode* node);
    void visitNumber(NumberNode* node);
    void visitChar(CharNode* node);
    void visitString(StringNode* node);
    void visitVar(VarNode* node);
    void visitUnaryOp(UnaryOpNode* node);
    void visitBinOp(BinOpNode* node);
    void visitType(TypeNode* node);
    void visitRange(RangeNode* node);
    void visitArrayType(ArrayTypeNode* node);
    void visitEnumeratedType(EnumeratedTypeNode* node);
    void visitRecordType(RecordTypeNode* node);
    void visitFieldPart(FieldPartNode* node);
    void visitVarDecl(VarDeclNode* node);
    void visitTypeDecl(TypeDeclNode* node);
    void visitConstDecl(ConstDeclNode* node);
    void visitParameter(ParameterNode* node);
    void visitFuncDecl(FuncDeclNode* node);
    void visitProcDecl(ProcDeclNode* node);
    void visitAssign(AssignNode* node);
    void visitIf(IfNode* node);
    void visitCaseBlock(CaseBlockNode* node);
    void visitCase(CaseNode* node);
    void visitWhile(WhileNode* node);
    void visitRepeat(RepeatNode* node);
    void visitFor(ForNode* node);
    void visitProcCall(ProcCallNode* node);
    void visitFuncCall(FuncCallNode* node);
    void visitArrayAccess(ArrayAccessNode* node);
    void visitRecordAccess(RecordAccessNode* node);
    bool hasErrors() const;
    void printErrors() const;
    void printErrors(std::ostream& output) const;
    void printSymbolTables() const;
    void printSymbolTables(std::ostream& output) const;
    const std::vector<std::string>& getErrors() const;
};
