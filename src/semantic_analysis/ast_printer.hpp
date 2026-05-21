#pragma once

#include "ast.hpp"
#include <ostream>
#include <string>

class ASTPrinter {
public:
    void print(ASTNode* root, std::ostream& out) const;

private:
    void printNode(ASTNode* node, std::ostream& out, int indentLevel) const;
    void printValue(ValueNode* node, std::ostream& out, int indentLevel) const;
    void printType(TypeNode* node, std::ostream& out, int indentLevel) const;
    void printChildren(ASTNode* node, std::ostream& out, int indentLevel) const;
    void writeAnnotation(ASTNode* node, std::ostream& out) const;
    void writeIndent(std::ostream& out, int indentLevel) const;
    std::string containerName(const ProcCallNode* node) const;
};
