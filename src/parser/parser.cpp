#include "parser/parser.hpp"

Parser::Parser(const std::vector<Token>& tokens) : tokens(Parser::cleanCommentTokens(tokens)), pos(0) {}

// Helper functions
Token Parser::currentToken() const {
    if ((size_t)pos < tokens.size()) {
        return tokens[pos];
    }
    throw std::runtime_error("Unexpected end of input");
}

bool Parser::check(ListToken type) const {
    return (size_t)pos < tokens.size() && tokens[pos].type == type;
}

Token Parser::peekToken(int offset) const {
    if ((size_t)(pos + offset) < tokens.size()) {
        return tokens[pos + offset];
    }
    throw std::runtime_error("Peek beyond end of input");
}

bool Parser::checkNext(ListToken type) const {
    return (size_t)(pos + 1) < tokens.size() && tokens[pos + 1].type == type;
}

TreeParser* Parser::match(ListToken expected) {
    if (Parser::check(expected)) {
        TreeParser* node = new TreeParser(Parser::currentToken().value);
        pos++;
        return node;
    }
    throw std::runtime_error("Expected token type " + std::to_string(expected) + " but found " + std::to_string(Parser::currentToken().type));
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
        throw std::runtime_error(std::string("Parsing error: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in ProgramHeader: ") + e.what());
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
                throw std::runtime_error("Invalid declaration order");
            }
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in DeclarationPart: ") + e.what());
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
                node->addChild(Parser::match(eql));
                node->addChild(Parser::Constant());
                node->addChild(Parser::match(semicolon));
            } while (Parser::check(ident));
        } else {
            throw std::runtime_error("Expected 'const' keyword");
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in ConstDeclaration: ") + e.what());
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
                throw std::runtime_error("Expected ident, intcon, or realcon in constant");
            }
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in Constant: ") + e.what());
    }
}

// <type-declaration> -> typesy (ident eql type semicolon)+
TreeParser* Parser::TypeDeclaration() {
    TreeParser* node = new TreeParser("<TypeDeclaration>");
    try {
        node->addChild(Parser::match(typesy));
        do {
            node->addChild(Parser::match(ident));
            node->addChild(Parser::match(eql));
            node->addChild(Parser::Type());
            node->addChild(Parser::match(semicolon));
        } while (Parser::check(ident));
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in TypeDeclaration: ") + e.what());
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
            // intcon/realcon/charcon/plus/minus -> kemungkinan besar range numerik (mis. 1..10)
            node->addChild(Parser::Range());
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in Type: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in ArrayType: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in Range: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in Enumerated: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in RecordType: ") + e.what());
    }
}

// <field-list> -> field-part (semicolon field-part)*
// Catatan: tolerir trailing semicolon sebelum endsy (umum di Pascal-style).
TreeParser* Parser::FieldList() {
    TreeParser* node = new TreeParser("<FieldList>");
    try {
        node->addChild(Parser::FieldPart());
        while (Parser::check(semicolon)) {
            node->addChild(Parser::match(semicolon));
            if (Parser::check(ident)) {
                node->addChild(Parser::FieldPart());
            } else {
                break; // trailing semicolon, keluar loop
            }
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in FieldList: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in FieldPart: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in VarDeclaration: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in IdentifierList: ") + e.what());
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
            throw std::runtime_error("Expected 'procedure' or 'function'");
        }
        return node;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error in SubprogramDeclaration: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in ProcedureDeclaration: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in FunctionDeclaration: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in Block: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in FormalParameterList: ") + e.what());
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
        throw std::runtime_error(std::string("Parsing error in ParameterGroup: ") + e.what());
    }
}
