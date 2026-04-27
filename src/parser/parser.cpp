#include "parser/parser.hpp"

Parser::Parser(const std::vector<Token>& tokens) : tokens(Parser::cleanCommentTokens(tokens)), pos(0) {}

// Helper functions
Token Parser::currentToken() const {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    throw std::runtime_error("Unexpected end of input");
}

bool Parser::check(ListToken type) const {
    return pos < tokens.size() && tokens[pos].type == type;;
}

TreeParser* Parser::match(ListToken expected) {
    if (check(expected)) {
        TreeParser* node = new TreeParser(currentToken().value);
        pos++;
        return node;
    }
    throw std::runtime_error("Expected token type " + std::to_string(expected) + " but found " + std::to_string(currentToken().type));
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
    return Program();
}

// <Program>
TreeParser* Parser::Program() {
    try {
        TreeParser* node = new TreeParser("<Program>");
        node->addChild(ProgramHeader());
        node->addChild(DeclarationPart());
        node->addChild(CompoundStatement());
        return node;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Parsing error: ") + e.what());
    }
}

// <ProgramHeader>
TreeParser* Parser::ProgramHeader() {
    try{
        TreeParser* node = new TreeParser("<ProgramHeader>");
        node->addChild(match(programsy));
        node->addChild(match(ident));
        node->addChild(match(semicolon));
        node->addChild(match(period));
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
        bool ConstDecl = false, TypeDecl = false, VarDecl = false, SubprogramDecl = false;
        while(check(constsy) || check(typesy) || check(varsy) || check(proceduresy)) {
            if(check(constsy) && !TypeDecl && !VarDecl && !SubprogramDecl) {
                node->addChild(Parser::ConstDeclaration());
                ConstDecl = true;
            }
            else if(check(typesy) && !VarDecl && !SubprogramDecl) {
                node->addChild(Parser::TypeDeclaration());
                TypeDecl = true;
                ConstDecl = true;
            }
            else if(check(varsy) && !SubprogramDecl) {
                node->addChild(Parser::VarDeclaration());
                VarDecl = true;
                TypeDecl = true;
                ConstDecl = true;
            }
            else if(check(proceduresy)) {
                node->addChild(Parser::SubprogramDeclaration());
                SubprogramDecl = true;
                VarDecl = true;
                TypeDecl = true;
                ConstDecl = true;
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

