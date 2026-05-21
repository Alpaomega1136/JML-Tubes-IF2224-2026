#include "ast_printer.hpp"

void ASTPrinter::print(ASTNode* root, std::ostream& out) const {
    printNode(root, out, 0);
}

void ASTPrinter::writeIndent(std::ostream& out, int indentLevel) const {
    if (indentLevel <= 0) {
        return;
    }

    for (int i = 0; i < indentLevel - 1; i++) {
        out << "|   ";
    }
    out << "|-- ";
}

void ASTPrinter::writeAnnotation(ASTNode* node, std::ostream& out) const {
    if (node == nullptr) {
        return;
    }
    if (node->semanticType != "unknown") {
        out << " [type=" << node->semanticType
            << ", tab=" << node->tabIndex
            << ", lev=" << node->lexicalLevel << "]";
    }
}

std::string ASTPrinter::containerName(const ProcCallNode* node) const {
    if (node == nullptr) {
        return "ProcCall";
    }

    if (node->name == "__declarations__") {
        return "Declarations";
    }

    if (node->name == "__block__") {
        return "Block";
    }

    return "ProcCall(name: '" + node->name + "')";
}

void ASTPrinter::printChildren(ASTNode* node, std::ostream& out, int indentLevel) const {
    if (node == nullptr) {
        return;
    }

    for (ASTNode* child : node->getChildren()) {
        printNode(child, out, indentLevel);
    }
}

void ASTPrinter::printType(TypeNode* node, std::ostream& out, int indentLevel) const {
    if (node == nullptr) {
        writeIndent(out, indentLevel);
        out << "<null-type>" << '\n';
        return;
    }

    if (RangeNode* range = dynamic_cast<RangeNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "RangeType";
        writeAnnotation(range, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "first:" << '\n';
        printValue(range->first, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "last:" << '\n';
        printValue(range->last, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "base:" << '\n';
        printType(range->rangeType, out, indentLevel + 2);
        return;
    }

    if (ArrayTypeNode* array = dynamic_cast<ArrayTypeNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "ArrayType";
        writeAnnotation(array, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "index:" << '\n';
        printType(array->idxType, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "element:" << '\n';
        printType(array->elType, out, indentLevel + 2);
        return;
    }

    if (RecordTypeNode* record = dynamic_cast<RecordTypeNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "RecordType";
        writeAnnotation(record, out);
        out << '\n';
        for (FieldPartNode* field : record->fieldList) {
            printNode(field, out, indentLevel + 1);
        }
        return;
    }

    writeIndent(out, indentLevel);
    out << "Type(name: '" << node->typeIdent << "')";
    writeAnnotation(node, out);
    out << '\n';
}

void ASTPrinter::printValue(ValueNode* node, std::ostream& out, int indentLevel) const {
    if (node == nullptr) {
        writeIndent(out, indentLevel);
        out << "<null-value>" << '\n';
        return;
    }

    if (NumberNode* number = dynamic_cast<NumberNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Number(value: " << number->num << ")";
        writeAnnotation(number, out);
        out << '\n';
        return;
    }

    if (CharNode* character = dynamic_cast<CharNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Char(value: '" << character->c << "')";
        writeAnnotation(character, out);
        out << '\n';
        return;
    }

    if (StringNode* stringNode = dynamic_cast<StringNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "String(value: '" << stringNode->str << "')";
        writeAnnotation(stringNode, out);
        out << '\n';
        return;
    }

    if (VarNode* variable = dynamic_cast<VarNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Var(name: '" << variable->name << "')";
        writeAnnotation(variable, out);
        out << '\n';
        return;
    }

    if (UnaryOpNode* unary = dynamic_cast<UnaryOpNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "UnaryOp(op: " << unary->op << ")";
        writeAnnotation(unary, out);
        out << '\n';
        printValue(unary->value, out, indentLevel + 1);
        return;
    }

    if (BinOpNode* binary = dynamic_cast<BinOpNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "BinOp(op: " << binary->op << ")";
        writeAnnotation(binary, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "left:" << '\n';
        printValue(binary->left, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "right:" << '\n';
        printValue(binary->right, out, indentLevel + 2);
        return;
    }

    if (FuncCallNode* call = dynamic_cast<FuncCallNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "FuncCall(name: '" << call->name << "')";
        writeAnnotation(call, out);
        out << '\n';
        for (ValueNode* arg : call->args) {
            printValue(arg, out, indentLevel + 1);
        }
        return;
    }

    if (ArrayAccessNode* array = dynamic_cast<ArrayAccessNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "ArrayAccess(name: '" << array->name << "')";
        writeAnnotation(array, out);
        out << '\n';
        printValue(array->idx, out, indentLevel + 1);
        return;
    }

    if (RecordAccessNode* record = dynamic_cast<RecordAccessNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "RecordAccess(name: '" << record->name
            << "', field: '" << record->fieldName << "')";
        writeAnnotation(record, out);
        out << '\n';
        return;
    }

    printNode(node, out, indentLevel);
}

void ASTPrinter::printNode(ASTNode* node, std::ostream& out, int indentLevel) const {
    if (node == nullptr) {
        writeIndent(out, indentLevel);
        out << "<null-node>" << '\n';
        return;
    }

    if (ValueNode* value = dynamic_cast<ValueNode*>(node)) {
        printValue(value, out, indentLevel);
        return;
    }

    if (ProgramNode* program = dynamic_cast<ProgramNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "ProgramNode(name: '" << program->name << "')";
        writeAnnotation(program, out);
        out << '\n';
        printChildren(program, out, indentLevel + 1);
        return;
    }

    if (VarDeclNode* varDecl = dynamic_cast<VarDeclNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "VarDecl(name: '" << varDecl->name << "')";
        writeAnnotation(varDecl, out);
        out << '\n';
        printType(varDecl->type, out, indentLevel + 1);
        return;
    }

    if (TypeDeclNode* typeDecl = dynamic_cast<TypeDeclNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "TypeDecl(name: '" << typeDecl->name << "')";
        writeAnnotation(typeDecl, out);
        out << '\n';
        printType(typeDecl->simpleType, out, indentLevel + 1);
        return;
    }

    if (ConstDeclNode* constDecl = dynamic_cast<ConstDeclNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "ConstDecl(name: '" << constDecl->name << "')";
        writeAnnotation(constDecl, out);
        out << '\n';
        printValue(constDecl->value, out, indentLevel + 1);
        return;
    }

    if (ParameterNode* parameter = dynamic_cast<ParameterNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Parameter(name: '" << parameter->name << "')";
        writeAnnotation(parameter, out);
        out << '\n';
        printType(parameter->type, out, indentLevel + 1);
        return;
    }

    if (FieldPartNode* field = dynamic_cast<FieldPartNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Field(name: '" << field->fieldIdent << "')";
        writeAnnotation(field, out);
        out << '\n';
        printType(field->fieldType, out, indentLevel + 1);
        return;
    }

    if (FuncDeclNode* funcDecl = dynamic_cast<FuncDeclNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "FunctionDecl(name: '" << funcDecl->name << "')";
        writeAnnotation(funcDecl, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "returnType:" << '\n';
        printType(funcDecl->returnType, out, indentLevel + 2);
        for (ParameterNode* parameter : funcDecl->parameterList) {
            printNode(parameter, out, indentLevel + 1);
        }
        printChildren(funcDecl, out, indentLevel + 1);
        return;
    }

    if (ProcDeclNode* procDecl = dynamic_cast<ProcDeclNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "ProcedureDecl(name: '" << procDecl->name << "')";
        writeAnnotation(procDecl, out);
        out << '\n';
        for (ParameterNode* parameter : procDecl->parameterList) {
            printNode(parameter, out, indentLevel + 1);
        }
        printChildren(procDecl, out, indentLevel + 1);
        return;
    }

    if (AssignNode* assign = dynamic_cast<AssignNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Assign";
        writeAnnotation(assign, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "target:" << '\n';
        printValue(assign->target, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "value:" << '\n';
        printValue(assign->value, out, indentLevel + 2);
        return;
    }

    if (IfNode* ifNode = dynamic_cast<IfNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "If";
        writeAnnotation(ifNode, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "condition:" << '\n';
        printValue(ifNode->condition, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "then:" << '\n';
        printNode(ifNode->then, out, indentLevel + 2);
        if (ifNode->elseThen != nullptr) {
            writeIndent(out, indentLevel + 1);
            out << "else:" << '\n';
            printNode(ifNode->elseThen, out, indentLevel + 2);
        }
        return;
    }

    if (CaseBlockNode* caseBlock = dynamic_cast<CaseBlockNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "CaseBlock";
        writeAnnotation(caseBlock, out);
        out << '\n';
        printValue(caseBlock->caseCondition, out, indentLevel + 1);
        printNode(caseBlock->statement, out, indentLevel + 1);
        return;
    }

    if (CaseNode* caseNode = dynamic_cast<CaseNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Case";
        writeAnnotation(caseNode, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "selector:" << '\n';
        printValue(caseNode->condition, out, indentLevel + 2);
        for (CaseBlockNode* caseBlock : caseNode->caseBlocks) {
            printNode(caseBlock, out, indentLevel + 1);
        }
        return;
    }

    if (WhileNode* whileNode = dynamic_cast<WhileNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "While";
        writeAnnotation(whileNode, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "condition:" << '\n';
        printValue(whileNode->condition, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "body:" << '\n';
        printNode(whileNode->statement, out, indentLevel + 2);
        return;
    }

    if (RepeatNode* repeatNode = dynamic_cast<RepeatNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "Repeat";
        writeAnnotation(repeatNode, out);
        out << '\n';
        writeIndent(out, indentLevel + 1);
        out << "body:" << '\n';
        printNode(repeatNode->statement, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "until:" << '\n';
        printValue(repeatNode->untilCondition, out, indentLevel + 2);
        return;
    }

    if (ForNode* forNode = dynamic_cast<ForNode*>(node)) {
        writeIndent(out, indentLevel);
        out << "For";
        writeAnnotation(forNode, out);
        out << '\n';
        printNode(forNode->traversalAssign, out, indentLevel + 1);
        writeIndent(out, indentLevel + 1);
        out << "to:" << '\n';
        printValue(forNode->to, out, indentLevel + 2);
        writeIndent(out, indentLevel + 1);
        out << "body:" << '\n';
        printNode(forNode->statement, out, indentLevel + 2);
        return;
    }

    if (ProcCallNode* call = dynamic_cast<ProcCallNode*>(node)) {
        writeIndent(out, indentLevel);
        out << containerName(call);
        writeAnnotation(call, out);
        out << '\n';
        for (ValueNode* arg : call->args) {
            printValue(arg, out, indentLevel + 1);
        }
        printChildren(call, out, indentLevel + 1);
        return;
    }

    writeIndent(out, indentLevel);
    out << "<unknown-node>" << '\n';
}
