#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "lexer/token.hpp"

struct TreeParser {
    std::string data;
    std::vector<TreeParser*> children;

    TreeParser(const std::string& data) : data(data) {}

    void addChild(TreeParser* child) {
        children.push_back(child);
    }
};

class Parser {
private:
    std::vector<Token> tokens;
    int pos;

    // ============== Helper ==============
    Token currentToken() const;
    Token peekToken(int offset = 1) const;
    bool check(ListToken) const;
    bool checkNext(ListToken type) const;
    TreeParser* match(ListToken expected);
    std::vector<Token> cleanCommentTokens(const std::vector<Token>& tokens) const;

    // ============== Program Root ==============
    TreeParser* Program();

    // ============== Program Header ==============
    TreeParser* ProgramHeader();

    // ============== Declaration Part ==============
    TreeParser* DeclarationPart();

    // const-declaration
    TreeParser* ConstDeclaration();
    TreeParser* Constant();

    // type-declaration
    TreeParser* TypeDeclaration();
    TreeParser* Type();
    TreeParser* ArrayType();
    TreeParser* Range();
    TreeParser* Enumerated();
    TreeParser* RecordType();
    TreeParser* FieldList();
    TreeParser* FieldPart();

    // var-declaration
    TreeParser* VarDeclaration();
    TreeParser* IdentifierList();

    // subprogram-declaration
    TreeParser* SubprogramDeclaration();
    TreeParser* ProcedureDeclaration();
    TreeParser* FunctionDeclaration();
    TreeParser* Block();
    TreeParser* FormalParameterList();
    TreeParser* ParameterGroup();

    // ============== Compound Statement ==============
    TreeParser* CompoundStatement();
    TreeParser* StatementList();
    TreeParser* Statement();

    // statements
    TreeParser* AssignmentStatement();
    TreeParser* IfStatement();
    TreeParser* CaseStatement();
    TreeParser* CaseBlock();
    TreeParser* WhileStatement();
    TreeParser* RepeatStatement();
    TreeParser* ForStatement();
    TreeParser* ProcedureFunctionCall();
    TreeParser* ParameterList();

    // expression
    TreeParser* Expression();
    TreeParser* SimpleExpression();
    TreeParser* Term();
    TreeParser* Factor();

    // operators
    TreeParser* RelationalOperator();
    TreeParser* AdditiveOperator();
    TreeParser* MultiplicativeOperator();

    // ============== Period ==============
    TreeParser* Period();

public:
    Parser(const std::vector<Token>& tokens);

    TreeParser* parse();
};