#include "semantic_analyzer.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

static std::string toLowerString(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

static bool isRealLiteral(const std::string& literal) {
    return literal.find('.') != std::string::npos ||
           literal.find('e') != std::string::npos ||
           literal.find('E') != std::string::npos;
}

void SemanticAnalyzer::analyze(ASTNode* root) {
    errors.clear();

    if (root == nullptr) {
        errors.push_back("Semantic error: AST root is null.");
        return;
    }

    ProgramNode* program = dynamic_cast<ProgramNode*>(root);
    if (program == nullptr) {
        errors.push_back("Semantic error: AST root is not a program node.");
        return;
    }

    analyzeProgram(program);
}

bool SemanticAnalyzer::hasErrors() const {
    return !errors.empty();
}

void SemanticAnalyzer::printErrors() const {
    printErrors(std::cout);
}

void SemanticAnalyzer::printErrors(std::ostream& output) const {
    for (const std::string& error : errors) {
        output << error << std::endl;
    }
}

void SemanticAnalyzer::printSymbolTables() const {
    printSymbolTables(std::cout);
}

void SemanticAnalyzer::printSymbolTables(std::ostream& output) const {
    symbolTable.printSpecTables(output);
}

const std::vector<std::string>& SemanticAnalyzer::getErrors() const {
    return errors;
}

void SemanticAnalyzer::analyzeProgram(ProgramNode* program) {
    if (program == nullptr) {
        return;
    }

    for (ASTNode* child : program->getChildren()) {
        ProcCallNode* container = dynamic_cast<ProcCallNode*>(child);
        if (container != nullptr && container->name == "__declarations__") {
            for (ASTNode* declaration : container->getChildren()) {
                analyzeDeclaration(declaration);
            }
        }
    }

    for (ASTNode* child : program->getChildren()) {
        ProcCallNode* container = dynamic_cast<ProcCallNode*>(child);
        if (container != nullptr && container->name == "__declarations__") {
            continue;
        }

        analyzeNode(child);
    }
}

void SemanticAnalyzer::analyzeNode(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    if (ProgramNode* program = dynamic_cast<ProgramNode*>(node)) {
        analyzeProgram(program);
        return;
    }

    if (ProcCallNode* procCall = dynamic_cast<ProcCallNode*>(node)) {
        if (procCall->name == "__declarations__") {
            for (ASTNode* child : procCall->getChildren()) {
                analyzeDeclaration(child);
            }
            return;
        }

        analyzeStatement(node);
        return;
    }

    if (dynamic_cast<VarDeclNode*>(node) != nullptr ||
        dynamic_cast<ConstDeclNode*>(node) != nullptr ||
        dynamic_cast<TypeDeclNode*>(node) != nullptr ||
        dynamic_cast<FuncDeclNode*>(node) != nullptr ||
        dynamic_cast<ProcDeclNode*>(node) != nullptr) {
        analyzeDeclaration(node);
        return;
    }

    analyzeStatement(node);
}

void SemanticAnalyzer::analyzeDeclaration(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    if (VarDeclNode* varDecl = dynamic_cast<VarDeclNode*>(node)) {
        declareOrReport({
            varDecl->name,
            SymbolKind::Variable,
            typeNameFromTypeNode(varDecl->type),
            symbolTable.currentLevel()
        });
        return;
    }

    if (ConstDeclNode* constDecl = dynamic_cast<ConstDeclNode*>(node)) {
        declareOrReport({
            constDecl->name,
            SymbolKind::Constant,
            typeNameFromValueNode(constDecl->value),
            symbolTable.currentLevel()
        });
        return;
    }

    if (TypeDeclNode* typeDecl = dynamic_cast<TypeDeclNode*>(node)) {
        declareOrReport({
            typeDecl->name,
            SymbolKind::Type,
            typeNameFromTypeNode(typeDecl->simpleType),
            symbolTable.currentLevel()
        });
        return;
    }

    if (FuncDeclNode* funcDecl = dynamic_cast<FuncDeclNode*>(node)) {
        analyzeFuncDeclaration(funcDecl);
        return;
    }

    if (ProcDeclNode* procDecl = dynamic_cast<ProcDeclNode*>(node)) {
        analyzeProcDeclaration(procDecl);
        return;
    }
}

void SemanticAnalyzer::analyzeProcDeclaration(ProcDeclNode* procDecl) {
    if (procDecl == nullptr) {
        return;
    }

    declareOrReport({
        procDecl->name,
        SymbolKind::Procedure,
        "procedure",
        symbolTable.currentLevel(),
        collectParameterTypes(procDecl->parameterList),
        collectParameterNames(procDecl->parameterList)
    });

    symbolTable.enterScope();
    declareParameters(procDecl->parameterList);
    analyzeSubprogramChildren(procDecl);
    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeFuncDeclaration(FuncDeclNode* funcDecl) {
    if (funcDecl == nullptr) {
        return;
    }

    std::string returnType = typeNameFromTypeNode(funcDecl->returnType);
    declareOrReport({
        funcDecl->name,
        SymbolKind::Function,
        returnType,
        symbolTable.currentLevel(),
        collectParameterTypes(funcDecl->parameterList),
        collectParameterNames(funcDecl->parameterList)
    });

    symbolTable.enterScope();

    // Pascal functions assign to their own name to set the return value.
    declareOrReport({
        funcDecl->name,
        SymbolKind::Variable,
        returnType,
        symbolTable.currentLevel()
    });

    declareParameters(funcDecl->parameterList);
    analyzeSubprogramChildren(funcDecl);
    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeSubprogramChildren(ASTNode* subprogramNode) {
    if (subprogramNode == nullptr) {
        return;
    }

    for (ASTNode* child : subprogramNode->getChildren()) {
        ProcCallNode* container = dynamic_cast<ProcCallNode*>(child);
        if (container != nullptr && container->name == "__declarations__") {
            for (ASTNode* declaration : container->getChildren()) {
                analyzeDeclaration(declaration);
            }
        }
    }

    for (ASTNode* child : subprogramNode->getChildren()) {
        ProcCallNode* container = dynamic_cast<ProcCallNode*>(child);
        if (container != nullptr && container->name == "__declarations__") {
            continue;
        }

        analyzeNode(child);
    }
}

void SemanticAnalyzer::declareParameters(const std::vector<ParameterNode*>& parameters) {
    for (ParameterNode* parameter : parameters) {
        if (parameter == nullptr) {
            continue;
        }

        declareOrReport({
            parameter->name,
            SymbolKind::Parameter,
            typeNameFromTypeNode(parameter->type),
            symbolTable.currentLevel()
        });
    }
}

std::vector<std::string> SemanticAnalyzer::collectParameterTypes(const std::vector<ParameterNode*>& parameters) const {
    std::vector<std::string> parameterTypes;
    for (ParameterNode* parameter : parameters) {
        if (parameter == nullptr) {
            continue;
        }

        parameterTypes.push_back(typeNameFromTypeNode(parameter->type));
    }

    return parameterTypes;
}

std::vector<std::string> SemanticAnalyzer::collectParameterNames(const std::vector<ParameterNode*>& parameters) const {
    std::vector<std::string> parameterNames;
    for (ParameterNode* parameter : parameters) {
        if (parameter == nullptr) {
            continue;
        }

        parameterNames.push_back(parameter->name);
    }

    return parameterNames;
}

void SemanticAnalyzer::analyzeStatement(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    if (AssignNode* assign = dynamic_cast<AssignNode*>(node)) {
        std::string targetType = "unknown";

        if (assign->target != nullptr) {
            SymbolEntry* target = symbolTable.lookup(assign->target->name);
            if (target == nullptr) {
                checkIdentifierDeclared(assign->target->name);
            } else {
                targetType = target->typeName;
            }
        }

        std::string valueType = inferExpressionType(assign->value);
        if (!isAssignmentCompatible(targetType, valueType)) {
            std::string targetName = (assign->target == nullptr) ? "<unknown>" : assign->target->name;
            errors.push_back(
                "Semantic error: cannot assign value of type '" + valueType +
                "' to variable '" + targetName +
                "' of type '" + targetType + "'."
            );
        }

        return;
    }

    if (IfNode* ifNode = dynamic_cast<IfNode*>(node)) {
        std::string conditionType = inferExpressionType(ifNode->condition);
        if (conditionType != "unknown" && conditionType != "boolean") {
            errors.push_back(
                "Semantic error: if condition must be boolean, got '" +
                conditionType + "'."
            );
        }
        analyzeNode(ifNode->then);
        analyzeNode(ifNode->elseThen);
        return;
    }

    if (WhileNode* whileNode = dynamic_cast<WhileNode*>(node)) {
        std::string conditionType = inferExpressionType(whileNode->condition);
        if (conditionType != "unknown" && conditionType != "boolean") {
            errors.push_back(
                "Semantic error: while condition must be boolean, got '" +
                conditionType + "'."
            );
        }
        analyzeNode(whileNode->statement);
        return;
    }

    if (RepeatNode* repeatNode = dynamic_cast<RepeatNode*>(node)) {
        analyzeNode(repeatNode->statement);
        std::string conditionType = inferExpressionType(repeatNode->untilCondition);
        if (conditionType != "unknown" && conditionType != "boolean") {
            errors.push_back(
                "Semantic error: repeat-until condition must be boolean, got '" +
                conditionType + "'."
            );
        }
        return;
    }

    if (ForNode* forNode = dynamic_cast<ForNode*>(node)) {
        std::string loopVariableName = "";
        std::string loopVariableType = "unknown";
        std::string startType = "unknown";

        if (forNode->traversalAssign != nullptr) {
            if (forNode->traversalAssign->target != nullptr) {
                loopVariableName = forNode->traversalAssign->target->name;
                SymbolEntry* loopVariable = symbolTable.lookup(loopVariableName);
                if (loopVariable == nullptr) {
                    checkIdentifierDeclared(loopVariableName);
                } else {
                    loopVariableType = loopVariable->typeName;
                    if (loopVariableType != "unknown" && loopVariableType != "integer") {
                        errors.push_back(
                            "Semantic error: for loop variable '" + loopVariableName +
                            "' must be integer, got '" + loopVariableType + "'."
                        );
                    }
                }
            }

            startType = inferExpressionType(forNode->traversalAssign->value);
            if (startType != "unknown" && startType != "integer") {
                errors.push_back(
                    "Semantic error: for loop start expression must be integer, got '" +
                    startType + "'."
                );
            }
        }

        std::string endType = inferExpressionType(forNode->to);
        if (endType != "unknown" && endType != "integer") {
            errors.push_back(
                "Semantic error: for loop end expression must be integer, got '" +
                endType + "'."
            );
        }

        analyzeNode(forNode->statement);
        return;
    }

    if (CaseNode* caseNode = dynamic_cast<CaseNode*>(node)) {
        inferExpressionType(caseNode->condition);
        for (CaseBlockNode* caseBlock : caseNode->caseBlocks) {
            analyzeNode(caseBlock);
        }
        return;
    }

    if (CaseBlockNode* caseBlock = dynamic_cast<CaseBlockNode*>(node)) {
        inferExpressionType(caseBlock->caseCondition);
        analyzeNode(caseBlock->statement);
        return;
    }

    if (ProcCallNode* procCall = dynamic_cast<ProcCallNode*>(node)) {
        if (isContainerNode(procCall)) {
            for (ASTNode* child : procCall->getChildren()) {
                analyzeNode(child);
            }
            return;
        }

        SymbolEntry* entry = symbolTable.lookup(procCall->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(procCall->name);
            for (ValueNode* arg : procCall->args) {
                inferExpressionType(arg);
            }
            return;
        }

        if (entry->kind != SymbolKind::Procedure && entry->kind != SymbolKind::Function) {
            errors.push_back(
                "Semantic error: identifier '" + procCall->name +
                "' is not callable."
            );
            for (ValueNode* arg : procCall->args) {
                inferExpressionType(arg);
            }
            return;
        }

        checkCallArguments(procCall->name, procCall->args, entry);
        return;
    }

    for (ASTNode* child : node->getChildren()) {
        analyzeNode(child);
    }
}

void SemanticAnalyzer::analyzeExpression(ValueNode* expr) {
    inferExpressionType(expr);
}

void SemanticAnalyzer::declareOrReport(const SymbolEntry& entry) {
    if (!symbolTable.declareSymbol(entry)) {
        errors.push_back(
            "Semantic error: identifier '" + entry.name +
            "' is already declared in this scope."
        );
    }
}

void SemanticAnalyzer::checkIdentifierDeclared(const std::string& name) {
    if (name.empty()) {
        return;
    }

    if (symbolTable.lookup(name) == nullptr) {
        std::string message = "Semantic error: identifier '" + name + "' is not declared.";
        if (!hasError(message)) {
            errors.push_back(message);
        }
    }
}

bool SemanticAnalyzer::hasError(const std::string& message) const {
    return std::find(errors.begin(), errors.end(), message) != errors.end();
}

bool SemanticAnalyzer::isContainerNode(ProcCallNode* callNode) const {
    if (callNode == nullptr) {
        return false;
    }

    return callNode->name == "__declarations__" || callNode->name == "__block__";
}

bool SemanticAnalyzer::isVariadicBuiltin(const std::string& name) const {
    return name == "readln" || name == "writeln";
}

bool SemanticAnalyzer::isNumericType(const std::string& typeName) const {
    return typeName == "integer" || typeName == "real";
}

bool SemanticAnalyzer::isSimpleAssignableType(const std::string& typeName) const {
    return typeName == "integer" ||
           typeName == "real" ||
           typeName == "char" ||
           typeName == "string" ||
           typeName == "boolean";
}

bool SemanticAnalyzer::isAssignmentCompatible(const std::string& targetType, const std::string& valueType) const {
    if (targetType == "unknown" || valueType == "unknown") {
        return true;
    }

    if (!isSimpleAssignableType(targetType) || !isSimpleAssignableType(valueType)) {
        return true;
    }

    if (targetType == valueType) {
        return true;
    }

    return targetType == "real" && valueType == "integer";
}

bool SemanticAnalyzer::isComparisonCompatible(const std::string& leftType, const std::string& rightType) const {
    if (leftType == "unknown" || rightType == "unknown") {
        return true;
    }

    if (leftType == rightType) {
        return true;
    }

    return isNumericType(leftType) && isNumericType(rightType);
}

void SemanticAnalyzer::checkCallArguments(const std::string& callName, const std::vector<ValueNode*>& args, SymbolEntry* entry) {
    if (entry == nullptr) {
        return;
    }

    if (isVariadicBuiltin(callName)) {
        for (ValueNode* arg : args) {
            inferExpressionType(arg);
        }
        return;
    }

    int expectedCount = static_cast<int>(entry->parameterTypes.size());
    int actualCount = static_cast<int>(args.size());

    if (expectedCount != actualCount) {
        std::string callableKind = (entry->kind == SymbolKind::Function) ? "function" : "procedure";
        errors.push_back(
            "Semantic error: " + callableKind + " '" + callName +
            "' expects " + std::to_string(expectedCount) +
            " argument(s), got " + std::to_string(actualCount) + "."
        );

        for (ValueNode* arg : args) {
            inferExpressionType(arg);
        }
        return;
    }

    for (int i = 0; i < actualCount; i++) {
        std::string expectedType = entry->parameterTypes[i];
        std::string actualType = inferExpressionType(args[i]);

        if (!isAssignmentCompatible(expectedType, actualType)) {
            errors.push_back(
                "Semantic error: argument " + std::to_string(i + 1) +
                " of '" + callName + "' expects '" + expectedType +
                "', got '" + actualType + "'."
            );
        }
    }
}

std::string SemanticAnalyzer::inferExpressionType(ValueNode* expr) {
    if (expr == nullptr) {
        return "unknown";
    }

    if (NumberNode* number = dynamic_cast<NumberNode*>(expr)) {
        return isRealLiteral(number->num) ? "real" : "integer";
    }

    if (dynamic_cast<StringNode*>(expr) != nullptr) {
        return "string";
    }

    if (dynamic_cast<CharNode*>(expr) != nullptr) {
        return "char";
    }

    if (VarNode* var = dynamic_cast<VarNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(var->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(var->name);
            return "unknown";
        }
        return entry->typeName;
    }

    if (UnaryOpNode* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        std::string op = toLowerString(unary->op);
        std::string valueType = inferExpressionType(unary->value);

        if (op == "plus" || op == "+" || op == "minus" || op == "-") {
            if (valueType != "unknown" && !isNumericType(valueType)) {
                errors.push_back(
                    "Semantic error: operator '" + unary->op +
                    "' requires numeric operand, got '" + valueType + "'."
                );
                return "unknown";
            }
            return valueType;
        }

        if (op == "not" || op == "notsy") {
            if (valueType != "unknown" && valueType != "boolean") {
                errors.push_back(
                    "Semantic error: operator 'not' requires boolean operand, got '" +
                    valueType + "'."
                );
                return "unknown";
            }
            return valueType == "unknown" ? "unknown" : "boolean";
        }

        return "unknown";
    }

    if (BinOpNode* binOp = dynamic_cast<BinOpNode*>(expr)) {
        std::string op = toLowerString(binOp->op);
        std::string leftType = inferExpressionType(binOp->left);
        std::string rightType = inferExpressionType(binOp->right);

        if (op == "plus" || op == "+" || op == "minus" || op == "-" ||
            op == "times" || op == "*") {
            if ((leftType != "unknown" && !isNumericType(leftType)) ||
                (rightType != "unknown" && !isNumericType(rightType))) {
                errors.push_back(
                    "Semantic error: operator '" + binOp->op +
                    "' requires numeric operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                return "unknown";
            }
            if (leftType == "unknown" || rightType == "unknown") {
                return "unknown";
            }
            return (leftType == "real" || rightType == "real") ? "real" : "integer";
        }

        if (op == "rdiv" || op == "/") {
            if ((leftType != "unknown" && !isNumericType(leftType)) ||
                (rightType != "unknown" && !isNumericType(rightType))) {
                errors.push_back(
                    "Semantic error: operator '" + binOp->op +
                    "' requires numeric operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                return "unknown";
            }
            return (leftType == "unknown" || rightType == "unknown") ? "unknown" : "real";
        }

        if (op == "idiv" || op == "div" || op == "imod" || op == "mod") {
            if ((leftType != "unknown" && leftType != "integer") ||
                (rightType != "unknown" && rightType != "integer")) {
                errors.push_back(
                    "Semantic error: operator '" + binOp->op +
                    "' requires integer operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                return "unknown";
            }
            return (leftType == "unknown" || rightType == "unknown") ? "unknown" : "integer";
        }

        if (op == "andsy" || op == "and" || op == "orsy" || op == "or") {
            if ((leftType != "unknown" && leftType != "boolean") ||
                (rightType != "unknown" && rightType != "boolean")) {
                errors.push_back(
                    "Semantic error: operator '" + binOp->op +
                    "' requires boolean operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                return "unknown";
            }
            return (leftType == "unknown" || rightType == "unknown") ? "unknown" : "boolean";
        }

        if (op == "eql" || op == "=" || op == "neq" || op == "<>" ||
            op == "lss" || op == "<" || op == "leq" || op == "<=" ||
            op == "gtr" || op == ">" || op == "geq" || op == ">=") {
            if (!isComparisonCompatible(leftType, rightType)) {
                errors.push_back(
                    "Semantic error: operator '" + binOp->op +
                    "' requires compatible operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                return "unknown";
            }
            return (leftType == "unknown" || rightType == "unknown") ? "unknown" : "boolean";
        }

        return "unknown";
    }

    if (FuncCallNode* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(funcCall->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(funcCall->name);
            for (ValueNode* arg : funcCall->args) {
                inferExpressionType(arg);
            }
            return "unknown";
        }

        if (entry->kind == SymbolKind::Procedure) {
            errors.push_back(
                "Semantic error: procedure '" + funcCall->name +
                "' cannot be used as a function."
            );
            checkCallArguments(funcCall->name, funcCall->args, entry);
            return "unknown";
        }

        if (entry->kind != SymbolKind::Function) {
            errors.push_back(
                "Semantic error: identifier '" + funcCall->name +
                "' is not callable."
            );
            for (ValueNode* arg : funcCall->args) {
                inferExpressionType(arg);
            }
            return "unknown";
        }

        checkCallArguments(funcCall->name, funcCall->args, entry);
        return entry->typeName;
    }

    if (ArrayAccessNode* arrayAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(arrayAccess->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(arrayAccess->name);
        }
        inferExpressionType(arrayAccess->idx);
        return entry == nullptr ? "unknown" : entry->typeName;
    }

    if (RecordAccessNode* recordAccess = dynamic_cast<RecordAccessNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(recordAccess->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(recordAccess->name);
        }
        return entry == nullptr ? "unknown" : entry->typeName;
    }

    return "unknown";
}

std::string SemanticAnalyzer::typeNameFromTypeNode(TypeNode* typeNode) const {
    if (typeNode == nullptr) {
        return "unknown";
    }

    return typeNode->typeIdent;
}

std::string SemanticAnalyzer::typeNameFromValueNode(ValueNode* valueNode) const {
    if (valueNode == nullptr) {
        return "unknown";
    }

    if (dynamic_cast<StringNode*>(valueNode) != nullptr) {
        return "string";
    }

    if (dynamic_cast<CharNode*>(valueNode) != nullptr) {
        return "char";
    }

    if (NumberNode* number = dynamic_cast<NumberNode*>(valueNode)) {
        if (isRealLiteral(number->num)) {
            return "real";
        }
        return "integer";
    }

    if (UnaryOpNode* unary = dynamic_cast<UnaryOpNode*>(valueNode)) {
        std::string op = toLowerString(unary->op);
        if (op == "plus" || op == "+" || op == "minus" || op == "-" ||
            op == "not" || op == "notsy") {
            return typeNameFromValueNode(unary->value);
        }
    }

    return "unknown";
}
