#pragma once

#include "ast.hpp"
#include "symbol_table.hpp"
#include <ostream>
#include <string>
#include <vector>

class SemanticAnalyzer {
private:
    SymbolTable symbolTable;
    std::vector<std::string> errors;

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
    bool isContainerNode(ProcCallNode* callNode) const;
    bool isVariadicBuiltin(const std::string& name) const;
    bool isNumericType(const std::string& typeName) const;
    bool isSimpleAssignableType(const std::string& typeName) const;
    bool isAssignmentCompatible(const std::string& targetType, const std::string& valueType) const;
    bool isComparisonCompatible(const std::string& leftType, const std::string& rightType) const;
    void checkCallArguments(const std::string& callName, const std::vector<ValueNode*>& args, SymbolEntry* entry);
    std::string inferExpressionType(ValueNode* expr);
    std::string typeNameFromTypeNode(TypeNode* typeNode) const;
    std::string typeNameFromValueNode(ValueNode* valueNode) const;

public:
    void analyze(ASTNode* root);
    bool hasErrors() const;
    void printErrors() const;
    void printErrors(std::ostream& output) const;
    void printSymbolTables() const;
    void printSymbolTables(std::ostream& output) const;
    const std::vector<std::string>& getErrors() const;
};
