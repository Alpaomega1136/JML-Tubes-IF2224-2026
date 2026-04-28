#include "parser/parser.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>

static std::string tokenToParseTreeString(const Token& token) {
    switch (token.type) {
        case intcon:
            return "intcon(" + token.value + ")";
        case realcon:
            return "realcon(" + token.value + ")";
        case charcon:
            return "charcon(" + token.value + ")";
        case stringcon:
            return "stringcon(" + token.value + ")";
        case ident:
            return "ident(" + token.value + ")";
        case comment:
            return "comment(" + token.value + ")";
        case unknown:
            return "unknown(" + token.value + ")";
        case plus: return "plus";
        case minus: return "minus";
        case times: return "times";
        case idiv: return "idiv";
        case rdiv: return "rdiv";
        case imod: return "imod";
        case notsy: return "notsy";
        case andsy: return "andsy";
        case orsy: return "orsy";
        case eql: return "eql";
        case neq: return "neq";
        case gtr: return "gtr";
        case geq: return "geq";
        case lss: return "lss";
        case leq: return "leq";
        case lparent: return "lparent";
        case rparent: return "rparent";
        case lbrack: return "lbrack";
        case rbrack: return "rbrack";
        case comma: return "comma";
        case semicolon: return "semicolon";
        case period: return "period";
        case colon: return "colon";
        case becomes: return "becomes";
        case constsy: return "constsy";
        case typesy: return "typesy";
        case varsy: return "varsy";
        case functionsy: return "functionsy";
        case proceduresy: return "proceduresy";
        case arraysy: return "arraysy";
        case recordsy: return "recordsy";
        case programsy: return "programsy";
        case beginsy: return "beginsy";
        case endsy: return "endsy";
        case ifsy: return "ifsy";
        case thensy: return "thensy";
        case elsesy: return "elsesy";
        case casesy: return "casesy";
        case ofsy: return "ofsy";
        case whilesy: return "whilesy";
        case dosy: return "dosy";
        case repeatsy: return "repeatsy";
        case untilsy: return "untilsy";
        case forsy: return "forsy";
        case tosy: return "tosy";
        case downtosy: return "downtosy";
    }

    return "";
}

static std::string tokenToErrorString(ListToken type) {
    switch (type) {
        case intcon: return "intcon (integer literal)";
        case realcon: return "realcon (real literal)";
        case charcon: return "charcon (character literal)";
        case stringcon: return "stringcon (string literal)";
        case plus: return "plus (+)";
        case minus: return "minus (-)";
        case times: return "times (*)";
        case idiv: return "idiv (div)";
        case rdiv: return "rdiv (/)";
        case imod: return "imod (mod)";
        case notsy: return "notsy (not)";
        case andsy: return "andsy (and)";
        case orsy: return "orsy (or)";
        case eql: return "eql (==)";
        case neq: return "neq (<>)";
        case gtr: return "gtr (>)";
        case geq: return "geq (>=)";
        case lss: return "lss (<)";
        case leq: return "leq (<=)";
        case lparent: return "lparent (()";
        case rparent: return "rparent ())";
        case lbrack: return "lbrack ([)";
        case rbrack: return "rbrack (])";
        case comma: return "comma (,)";
        case semicolon: return "semicolon (;)";
        case period: return "period (.)";
        case colon: return "colon (:)";
        case becomes: return "becomes (:=)";
        case constsy: return "constsy (const)";
        case typesy: return "typesy (type)";
        case varsy: return "varsy (var)";
        case functionsy: return "functionsy (function)";
        case proceduresy: return "proceduresy (procedure)";
        case arraysy: return "arraysy (array)";
        case recordsy: return "recordsy (record)";
        case programsy: return "programsy (program)";
        case beginsy: return "beginsy (begin)";
        case endsy: return "endsy (end)";
        case ifsy: return "ifsy (if)";
        case thensy: return "thensy (then)";
        case elsesy: return "elsesy (else)";
        case casesy: return "casesy (case)";
        case ofsy: return "ofsy (of)";
        case whilesy: return "whilesy (while)";
        case dosy: return "dosy (do)";
        case repeatsy: return "repeatsy (repeat)";
        case untilsy: return "untilsy (until)";
        case forsy: return "forsy (for)";
        case tosy: return "tosy (to)";
        case downtosy: return "downtosy (downto)";
        case ident: return "ident (identifier)";
        case comment: return "comment";
        case unknown: return "unknown token";
    }

    return "unknown token";
}

static std::string tokenToErrorString(const Token& token) {
    if (token.type == ident || token.type == intcon || token.type == realcon ||
        token.type == charcon || token.type == stringcon || token.type == unknown) {
        return tokenToErrorString(token.type) + " with value '" + token.value + "'";
    }

    return tokenToErrorString(token.type);
}

static bool startsWith(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

Parser::Parser(const std::vector<Token>& tokens) : tokens(Parser::cleanCommentTokens(tokens)), pos(0) {}

// Helper functions
Token Parser::currentToken() const {
    if ((size_t)pos < tokens.size()) {
        return tokens[pos];
    }
    throw std::runtime_error("unexpected end of input");
}

bool Parser::check(ListToken type) const {
    return (size_t)pos < tokens.size() && tokens[pos].type == type;
}

Token Parser::peekToken(int offset) const {
    if ((size_t)(pos + offset) < tokens.size()) {
        return tokens[pos + offset];
    }
    throw std::runtime_error("no next token available");
}

bool Parser::checkNext(ListToken type) const {
    return (size_t)(pos + 1) < tokens.size() && tokens[pos + 1].type == type;
}

TreeParser* Parser::match(ListToken expected) {
    if (Parser::check(expected)) {
        TreeParser* node = new TreeParser(tokenToParseTreeString(Parser::currentToken()));
        pos++;
        return node;
    }

    std::string foundToken = (size_t)pos < tokens.size() ? tokenToErrorString(tokens[pos]) : "akhir input";
    throw std::runtime_error("expected " + tokenToErrorString(expected) + ", found " + foundToken);
}

std::vector<Token> Parser::cleanCommentTokens(const std::vector<Token>& tokens) const {
    std::vector<Token> cleanedTokens;
    for (const Token& token : tokens) {
        if (token.type != comment) {
            cleanedTokens.push_back(token);
        }
    }
    return cleanedTokens;
}

// Parsing functions
TreeParser* Parser::parse() {
    return Parser::Program();
}

// <Program>
TreeParser* Parser::Program() {
    try {
        TreeParser* node = new TreeParser("<Program>");
        node->addChild(Parser::ProgramHeader());
        node->addChild(Parser::DeclarationPart());
        node->addChild(Parser::CompoundStatement());
        node->addChild(Parser::match(period));
        return node;
    } catch (const std::runtime_error& e) {
        if (startsWith(e.what(), "Syntax error")) {
            throw;
        }
        throw std::runtime_error(std::string("Syntax error: ") + e.what());
    }
}

// <ProgramHeader>
TreeParser* Parser::ProgramHeader() {
    try{
        TreeParser* node = new TreeParser("<ProgramHeader>");
        node->addChild(Parser::match(programsy));
        node->addChild(Parser::match(ident));
        node->addChild(Parser::match(semicolon));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in ProgramHeader: ") + e.what());
    }
}

// <DeclarationPart>
TreeParser* Parser::DeclarationPart() {
    TreeParser* node = new TreeParser("<DeclarationPart>");
    try{
        bool TypeDecl = false, VarDecl = false, SubprogramDecl = false;
        while(Parser::check(constsy) || Parser::check(typesy) || Parser::check(varsy) || Parser::check(proceduresy) || Parser::check(functionsy)) {
            if(Parser::check(constsy) && !TypeDecl && !VarDecl && !SubprogramDecl) {
                node->addChild(Parser::ConstDeclaration());
            }
            else if(Parser::check(typesy) && !VarDecl && !SubprogramDecl) {
                node->addChild(Parser::TypeDeclaration());
                TypeDecl = true;
            }
            else if(Parser::check(varsy) && !SubprogramDecl) {
                node->addChild(Parser::VarDeclaration());
                VarDecl = true;
                TypeDecl = true;
            }
            else if(Parser::check(proceduresy) || Parser::check(functionsy)) {
                node->addChild(Parser::SubprogramDeclaration());
                SubprogramDecl = true;
                VarDecl = true;
                TypeDecl = true;
            }
            else {
        throw std::runtime_error("invalid declaration order: use const, type, var, then procedure/function");
            }
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in DeclarationPart: ") + e.what());
    }
}

// <const-declaration> -> constsy (ident eql constant semicolon)+
TreeParser* Parser::ConstDeclaration() {
    TreeParser* node = new TreeParser("<ConstDeclaration>");
    try{
        if(Parser::check(constsy)){
            node->addChild(Parser::match(constsy));
            do {
                node->addChild(Parser::match(ident));
                if (Parser::check(eql)) {
                    node->addChild(Parser::match(eql));
                } else {
                    node->addChild(Parser::match(becomes));
                }
                node->addChild(Parser::Constant());
                node->addChild(Parser::match(semicolon));
            } while (Parser::check(ident));
        } else {
            throw std::runtime_error("expected const keyword");
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in ConstDeclaration: ") + e.what());
    }
}

// <constant> -> charcon | stringcon | [(plus|minus)? (ident|intcon|realcon)]
TreeParser* Parser::Constant() {
    TreeParser* node = new TreeParser("<Constant>");
    try {
        if (Parser::check(charcon)) {
            node->addChild(Parser::match(charcon));
        } else if (Parser::check(stringcon)) {
            node->addChild(Parser::match(stringcon));
        } else {
            // (plus | minus)?
            if (Parser::check(plus)) {
                node->addChild(Parser::match(plus));
            } else if (Parser::check(minus)) {
                node->addChild(Parser::match(minus));
            }
            // (ident | intcon | realcon)
            if (Parser::check(ident)) {
                node->addChild(Parser::match(ident));
            } else if (Parser::check(intcon)) {
                node->addChild(Parser::match(intcon));
            } else if (Parser::check(realcon)) {
                node->addChild(Parser::match(realcon));
            } else {
                throw std::runtime_error("expected identifier, integer literal, or real literal in constant");
            }
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in Constant: ") + e.what());
    }
}

// <type-declaration> -> typesy (ident eql type semicolon)+
TreeParser* Parser::TypeDeclaration() {
    TreeParser* node = new TreeParser("<TypeDeclaration>");
    try {
        node->addChild(Parser::match(typesy));
        do {
            node->addChild(Parser::match(ident));
            if (Parser::check(eql)) {
                node->addChild(Parser::match(eql));
            } else {
                node->addChild(Parser::match(colon));
            }
            node->addChild(Parser::Type());
            node->addChild(Parser::match(semicolon));
        } while (Parser::check(ident));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in TypeDeclaration: ") + e.what());
    }
}

// <type> -> ident | array-type | range | enumerated | record-type
TreeParser* Parser::Type() {
    TreeParser* node = new TreeParser("<Type>");
    try {
        if (Parser::check(arraysy)) {
            node->addChild(Parser::ArrayType());
        } else if (Parser::check(recordsy)) {
            node->addChild(Parser::RecordType());
        } else if (Parser::check(lparent)) {
            // enumerated diawali '('
            node->addChild(Parser::Enumerated());
        } else if (Parser::check(ident)) {
            if (Parser::checkNext(period)) {
                node->addChild(Parser::Range());
            } else {
                node->addChild(Parser::match(ident));
            }
        } else {
            // intcon/realcon/charcon/plus/minus 
            node->addChild(Parser::Range());
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in Type: ") + e.what());
    }
}

// <array-type> -> arraysy lbrack (range | ident) rbrack ofsy type
TreeParser* Parser::ArrayType() {
    TreeParser* node = new TreeParser("<ArrayType>");
    try {
        node->addChild(Parser::match(arraysy));
        node->addChild(Parser::match(lbrack));
        if (Parser::check(ident) && Parser::checkNext(rbrack)) {
            node->addChild(Parser::match(ident));
        } else {
            node->addChild(Parser::Range());
        }
        node->addChild(Parser::match(rbrack));
        node->addChild(Parser::match(ofsy));
        node->addChild(Parser::Type());
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in ArrayType: ") + e.what());
    }
}

// <range> -> expression period period expression
TreeParser* Parser::Range() {
    TreeParser* node = new TreeParser("<Range>");
    try {
        node->addChild(Parser::Expression());
        node->addChild(Parser::match(period));
        node->addChild(Parser::match(period));
        node->addChild(Parser::Expression());
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in Range: ") + e.what());
    }
}

// <enumerated> -> lparent ident (comma ident)* rparent
TreeParser* Parser::Enumerated() {
    TreeParser* node = new TreeParser("<Enumerated>");
    try {
        node->addChild(Parser::match(lparent));
        node->addChild(Parser::match(ident));
        while (Parser::check(comma)) {
            node->addChild(Parser::match(comma));
            node->addChild(Parser::match(ident));
        }
        node->addChild(Parser::match(rparent));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in Enumerated: ") + e.what());
    }
}

// <record-type> -> recordsy field-list endsy
TreeParser* Parser::RecordType() {
    TreeParser* node = new TreeParser("<RecordType>");
    try {
        node->addChild(Parser::match(recordsy));
        node->addChild(Parser::FieldList());
        node->addChild(Parser::match(endsy));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in RecordType: ") + e.what());
    }
}

// <field-list> -> field-part (semicolon field-part)*
TreeParser* Parser::FieldList() {
    TreeParser* node = new TreeParser("<FieldList>");
    try {
        node->addChild(Parser::FieldPart());
        while (Parser::check(semicolon)) {
            node->addChild(Parser::match(semicolon));
            if (Parser::check(ident)) {
                node->addChild(Parser::FieldPart());
            } else {
                break;
            }
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in FieldList: ") + e.what());
    }
}

// <field-part> -> identifier-list colon type
TreeParser* Parser::FieldPart() {
    TreeParser* node = new TreeParser("<FieldPart>");
    try {
        node->addChild(Parser::IdentifierList());
        node->addChild(Parser::match(colon));
        node->addChild(Parser::Type());
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in FieldPart: ") + e.what());
    }
}

// <var-declaration> -> varsy (identifier-list colon type semicolon)+
TreeParser* Parser::VarDeclaration() {
    TreeParser* node = new TreeParser("<VarDeclaration>");
    try {
        node->addChild(Parser::match(varsy));
        do {
            node->addChild(Parser::IdentifierList());
            node->addChild(Parser::match(colon));
            node->addChild(Parser::Type());
            node->addChild(Parser::match(semicolon));
        } while (Parser::check(ident));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in VarDeclaration: ") + e.what());
    }
}

// <identifier-list> -> ident (comma ident)*
TreeParser* Parser::IdentifierList() {
    TreeParser* node = new TreeParser("<IdentifierList>");
    try {
        node->addChild(Parser::match(ident));
        while (Parser::check(comma)) {
            node->addChild(Parser::match(comma));
            node->addChild(Parser::match(ident));
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in IdentifierList: ") + e.what());
    }
}

// <subprogram-declaration> -> procedure-declaration | function-declaration
TreeParser* Parser::SubprogramDeclaration() {
    TreeParser* node = new TreeParser("<SubprogramDeclaration>");
    try {
        if (Parser::check(proceduresy)) {
            node->addChild(Parser::ProcedureDeclaration());
        } else if (Parser::check(functionsy)) {
            node->addChild(Parser::FunctionDeclaration());
        } else {
            throw std::runtime_error("expected procedure or function keyword");
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in SubprogramDeclaration: ") + e.what());
    }
}

// <procedure-declaration> -> proceduresy ident (formal-parameter-list)? semicolon block semicolon
TreeParser* Parser::ProcedureDeclaration() {
    TreeParser* node = new TreeParser("<ProcedureDeclaration>");
    try {
        node->addChild(Parser::match(proceduresy));
        node->addChild(Parser::match(ident));
        if (Parser::check(lparent)) {
            node->addChild(Parser::FormalParameterList());
        }
        node->addChild(Parser::match(semicolon));
        node->addChild(Parser::Block());
        node->addChild(Parser::match(semicolon));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in ProcedureDeclaration: ") + e.what());
    }
}

// <function-declaration> -> functionsy ident (formal-parameter-list)? colon ident semicolon block semicolon
TreeParser* Parser::FunctionDeclaration() {
    TreeParser* node = new TreeParser("<FunctionDeclaration>");
    try {
        node->addChild(Parser::match(functionsy));
        node->addChild(Parser::match(ident));
        if (Parser::check(lparent)) {
            node->addChild(Parser::FormalParameterList());
        }
        node->addChild(Parser::match(colon));
        node->addChild(Parser::match(ident));
        node->addChild(Parser::match(semicolon));
        node->addChild(Parser::Block());
        node->addChild(Parser::match(semicolon));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in FunctionDeclaration: ") + e.what());
    }
}

// <block> -> declaration-part compound-statement
TreeParser* Parser::Block() {
    TreeParser* node = new TreeParser("<Block>");
    try {
        node->addChild(Parser::DeclarationPart());
        node->addChild(Parser::CompoundStatement());
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in Block: ") + e.what());
    }
}

// <formal-parameter-list> -> lparent parameter-group (semicolon parameter-group)* rparent
TreeParser* Parser::FormalParameterList() {
    TreeParser* node = new TreeParser("<FormalParameterList>");
    try {
        node->addChild(Parser::match(lparent));
        node->addChild(Parser::ParameterGroup());
        while (Parser::check(semicolon)) {
            node->addChild(Parser::match(semicolon));
            node->addChild(Parser::ParameterGroup());
        }
        node->addChild(Parser::match(rparent));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in FormalParameterList: ") + e.what());
    }
}

// <parameter-group> -> identifier-list colon (ident | array-type)
TreeParser* Parser::ParameterGroup() {
    TreeParser* node = new TreeParser("<ParameterGroup>");
    try {
        if (Parser::check(varsy)) {
            node->addChild(Parser::match(varsy));
        }
        node->addChild(Parser::IdentifierList());
        node->addChild(Parser::match(colon));
        if (Parser::check(arraysy)) {
            node->addChild(Parser::ArrayType());
        } else {
            node->addChild(Parser::match(ident));
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in ParameterGroup: ") + e.what());
    }
}

// <compound-statement> -> beginsy statement-list endsy
TreeParser* Parser::CompoundStatement() {
    TreeParser* node = new TreeParser("<compound-statement>");
    try {
        node->addChild(match(beginsy));
        node->addChild(StatementList());
        node->addChild(match(endsy));
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <compound-statement>: ") + e.what());
    }
}

// <statement-list> -> statement (semicolon statement)*
TreeParser* Parser::StatementList() {
    TreeParser* node = new TreeParser("<statement-list>");
    try {
        node->addChild(Statement());
        while (check(semicolon)) {
            node->addChild(match(semicolon));
            node->addChild(Statement());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <statement-list>: ") + e.what());
    }
}
 
// <statement> -> assignment-statement | if-statement | case-statement
//              | while-statement | repeat-statement | for-statement
//              | procedure/function-call | compound-statement | (empty)
TreeParser* Parser::Statement() {
    TreeParser* node = new TreeParser("<statement>");
    try {
        if (check(ident)) {
            if (checkNext(becomes) || checkNext(period) || checkNext(lbrack)) {
                node->addChild(AssignmentStatement());
            } else {
                node->addChild(ProcedureFunctionCall());
            }
        } else if (check(ifsy)) {
            node->addChild(IfStatement());
        } else if (check(casesy)) {
            node->addChild(CaseStatement());
        } else if (check(whilesy)) {
            node->addChild(WhileStatement());
        } else if (check(repeatsy)) {
            node->addChild(RepeatStatement());
        } else if (check(forsy)) {
            node->addChild(ForStatement());
        } else if (check(beginsy)) {
            node->addChild(CompoundStatement());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <statement>: ") + e.what());
    }
}
 
// <assignment-statement> -> ident becomes expression
TreeParser* Parser::AssignmentStatement() {
    TreeParser* node = new TreeParser("<assignment-statement>");
    try {
        node->addChild(Variable());
        node->addChild(match(becomes));
        node->addChild(Expression());
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <assignment-statement>: ") + e.what());
    }
}
 
// <if-statement> -> ifsy expression thensy statement (elsesy statement)?
TreeParser* Parser::IfStatement() {
    TreeParser* node = new TreeParser("<if-statement>");
    try {
        node->addChild(match(ifsy));
        node->addChild(Expression());
        node->addChild(match(thensy));
        node->addChild(Statement());
        if (check(elsesy)) {
            node->addChild(match(elsesy));
            node->addChild(Statement());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <if-statement>: ") + e.what());
    }
}
 
// <case-statement> -> casesy expression ofsy case-block endsy
TreeParser* Parser::CaseStatement() {
    TreeParser* node = new TreeParser("<case-statement>");
    try {
        node->addChild(match(casesy));
        node->addChild(Expression());
        node->addChild(match(ofsy));
        node->addChild(CaseBlock());
        node->addChild(match(endsy));
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <case-statement>: ") + e.what());
    }
}
 
// <case-block> -> constant (comma constant)* colon statement 
//                (semicolon case-block?)*
TreeParser* Parser::CaseBlock() {
    TreeParser* node = new TreeParser("<case-block>");
    try {
        node->addChild(Constant());
        while (check(comma)) {
            node->addChild(match(comma));
            node->addChild(Constant());
        }
        node->addChild(match(colon));
        node->addChild(Statement());
 
        while (check(semicolon)) {
            node->addChild(match(semicolon));
            bool hasNextCase = check(ident) || check(intcon) || check(realcon) ||
                               check(charcon) || check(stringcon) ||
                               check(plus) || check(minus);
            if (hasNextCase && !check(endsy)) {
                node->addChild(CaseBlock());
            }
            if (!hasNextCase) break;
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <case-block>: ") + e.what());
    }
}
 
// <while-statement> -> whilesy expression dosy statement
TreeParser* Parser::WhileStatement() {
    TreeParser* node = new TreeParser("<while-statement>");
    try {
        node->addChild(match(whilesy));
        node->addChild(Expression());
        node->addChild(match(dosy));
        node->addChild(Statement());
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <while-statement>: ") + e.what());
    }
}
 
// <repeat-statement> -> repeatsy statement-list untilsy expression
TreeParser* Parser::RepeatStatement() {
    TreeParser* node = new TreeParser("<repeat-statement>");
    try {
        node->addChild(match(repeatsy));
        node->addChild(StatementList());
        node->addChild(match(untilsy));
        node->addChild(Expression());
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <repeat-statement>: ") + e.what());
    }
}
 
// <for-statement> -> forsy ident becomes expression (tosy|downtosy)
//                    expression dosy statement
TreeParser* Parser::ForStatement() {
    TreeParser* node = new TreeParser("<for-statement>");
    try {
        node->addChild(match(forsy));
        node->addChild(match(ident));
        node->addChild(match(becomes));
        node->addChild(Expression());
        if (check(tosy))           node->addChild(match(tosy));
        else if (check(downtosy))  node->addChild(match(downtosy));
        else throw std::runtime_error("expected to or downto keyword in <for-statement>");
        node->addChild(Expression());
        node->addChild(match(dosy));
        node->addChild(Statement());
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <for-statement>: ") + e.what());
    }
}
 
// <procedure/function-call> -> ident (lparent parameter-list rparent)?
TreeParser* Parser::ProcedureFunctionCall() {
    TreeParser* node = new TreeParser("<procedure/function-call>");
    try {
        node->addChild(match(ident));
        if (check(lparent)) {
            node->addChild(match(lparent));
            if (!check(rparent)) {
                node->addChild(ParameterList());
            }
            node->addChild(match(rparent));
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <procedure/function-call>: ") + e.what());
    }
}
 
// <parameter-list> -> expression (comma expression)*
TreeParser* Parser::ParameterList() {
    TreeParser* node = new TreeParser("<parameter-list>");
    try {
        node->addChild(Expression());
        while (check(comma)) {
            node->addChild(match(comma));
            node->addChild(Expression());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <parameter-list>: ") + e.what());
    }
}

// <variable> -> ident (period ident | lbrack expression rbrack)*
TreeParser* Parser::Variable() {
    TreeParser* node = new TreeParser("<variable>");
    try {
        node->addChild(match(ident));
        while ((check(period) && checkNext(ident)) || check(lbrack)) {
            if (check(period)) {
                node->addChild(match(period));
                node->addChild(match(ident));
            } else {
                node->addChild(match(lbrack));
                node->addChild(Expression());
                node->addChild(match(rbrack));
            }
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <variable>: ") + e.what());
    }
}
 
// <expression> -> simple-expression (relational-operator simple-expression)?
TreeParser* Parser::Expression() {
    TreeParser* node = new TreeParser("<expression>");
    try {
        node->addChild(SimpleExpression());
        if (check(eql) || check(neq) || check(gtr) ||
            check(geq) || check(lss) || check(leq)) {
            node->addChild(RelationalOperator());
            node->addChild(SimpleExpression());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <expression>: ") + e.what());
    }
}
 
// <simple-expression> -> (plus|minus)? term (additive-operator term)*
TreeParser* Parser::SimpleExpression() {
    TreeParser* node = new TreeParser("<simple-expression>");
    try {
        if (check(plus))       node->addChild(match(plus));
        else if (check(minus)) node->addChild(match(minus));
 
        node->addChild(Term());
        while (check(plus) || check(minus) || check(orsy)) {
            node->addChild(AdditiveOperator());
            node->addChild(Term());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <simple-expression>: ") + e.what());
    }
}
 
// <term> -> factor (multiplicative-operator factor)*
TreeParser* Parser::Term() {
    TreeParser* node = new TreeParser("<term>");
    try {
        node->addChild(Factor());
        while (check(times) || check(rdiv) || check(idiv) ||
               check(imod) || check(andsy)) {
            node->addChild(MultiplicativeOperator());
            node->addChild(Factor());
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <term>: ") + e.what());
    }
}
 
// <factor> -> ident | intcon | realcon | charcon | stringcon
//           | lparent expression rparent
//           | notsy factor
//           | procedure/function-call   (ident '(' ... ')')
TreeParser* Parser::Factor() {
    TreeParser* node = new TreeParser("<factor>");
    try {
        if (check(ident)) {
            if (checkNext(lparent)) node->addChild(ProcedureFunctionCall());
            else                    node->addChild(Variable());
        } else if (check(intcon)) {
            node->addChild(match(intcon));
        } else if (check(realcon)) {
            node->addChild(match(realcon));
        } else if (check(charcon)) {
            node->addChild(match(charcon));
        } else if (check(stringcon)) {
            node->addChild(match(stringcon));
        } else if (check(lparent)) {
            node->addChild(match(lparent));
            node->addChild(Expression());
            node->addChild(match(rparent));
        } else if (check(notsy)) {
            node->addChild(match(notsy));
            node->addChild(Factor());
        } else {
            throw std::runtime_error(
                "invalid token in <factor>: found " +
                tokenToErrorString(currentToken()));
        }
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <factor>: ") + e.what());
    }
}
 
// <relational-operator> -> eql | neq | gtr | geq | lss | leq
TreeParser* Parser::RelationalOperator() {
    TreeParser* node = new TreeParser("<relational-operator>");
    try {
        if      (check(eql)) node->addChild(match(eql));
        else if (check(neq)) node->addChild(match(neq));
        else if (check(gtr)) node->addChild(match(gtr));
        else if (check(geq)) node->addChild(match(geq));
        else if (check(lss)) node->addChild(match(lss));
        else if (check(leq)) node->addChild(match(leq));
        else throw std::runtime_error("expected relational operator: ==, <>, >, >=, <, or <=");
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <relational-operator>: ") + e.what());
    }
}
 
// <additive-operator> -> plus | minus | orsy
TreeParser* Parser::AdditiveOperator() {
    TreeParser* node = new TreeParser("<additive-operator>");
    try {
        if      (check(plus))  node->addChild(match(plus));
        else if (check(minus)) node->addChild(match(minus));
        else if (check(orsy))  node->addChild(match(orsy));
        else throw std::runtime_error("expected additive operator: +, -, or or");
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <additive-operator>: ") + e.what());
    }
}
 
// <multiplicative-operator> -> times | rdiv | idiv | imod | andsy
TreeParser* Parser::MultiplicativeOperator() {
    TreeParser* node = new TreeParser("<multiplicative-operator>");
    try {
        if      (check(times)) node->addChild(match(times));
        else if (check(rdiv))  node->addChild(match(rdiv));
        else if (check(idiv))  node->addChild(match(idiv));
        else if (check(imod))  node->addChild(match(imod));
        else if (check(andsy)) node->addChild(match(andsy));
        else throw std::runtime_error("expected multiplicative operator: *, /, div, mod, or and");
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Syntax error in <multiplicative-operator>: ") + e.what());
    }
}

static void printParseTreeLine(TreeParser* node, std::ostream& output, const std::string& prefix, bool isLast) {
    output << prefix;
    if (isLast) {
        output << "└── ";
    } else {
        output << "├── ";
    }
    output << node->data << '\n';

    std::string childPrefix = prefix;
    if (isLast) {
        childPrefix += "    ";
    } else {
        childPrefix += "│   ";
    }
    for (size_t i = 0; i < node->children.size(); ++i) {
        printParseTreeLine(node->children[i], output, childPrefix, i == node->children.size() - 1);
    }
}

static void writeParseTree(std::ostream& output, TreeParser* root) {
    output << root->data << '\n';
    for (size_t i = 0; i < root->children.size(); ++i) {
        printParseTreeLine(root->children[i], output, "", i == root->children.size() - 1);
    }
}

void Parser::printParseTree(TreeParser* node, const std::string& path) const {
    if (node == nullptr) {
        throw std::runtime_error("empty parse tree");
    }

    std::filesystem::path outputPath(path);
    if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open output file: " + path);
    }

    writeParseTree(std::cout, node);
    writeParseTree(file, node);
}
