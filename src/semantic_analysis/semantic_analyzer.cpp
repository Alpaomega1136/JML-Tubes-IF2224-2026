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
    typeRegistry.clear();
    initializeBuiltinTypes();

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

std::string SemanticAnalyzer::normalizeName(const std::string& text) const {
    return toLowerString(text);
}

void SemanticAnalyzer::addError(const std::string& message) {
    if (!hasError(message)) {
        errors.push_back(message);
    }
}

void SemanticAnalyzer::initializeBuiltinTypes() {
    const std::vector<std::string> builtinTypes = {
        "integer", "real", "char", "boolean", "string", "procedure", "unknown"
    };

    for (const std::string& typeName : builtinTypes) {
        TypeInfo info;
        info.kind = "simple";
        info.baseType = typeName;
        typeRegistry[typeName] = info;
    }
}

bool SemanticAnalyzer::extractOrdinalValue(ValueNode* valueNode, int& value) const {
    if (valueNode == nullptr) {
        return false;
    }

    if (NumberNode* number = dynamic_cast<NumberNode*>(valueNode)) {
        if (isRealLiteral(number->num)) {
            return false;
        }

        try {
            value = std::stoi(number->num);
            return true;
        } catch (...) {
            return false;
        }
    }

    if (CharNode* character = dynamic_cast<CharNode*>(valueNode)) {
        value = static_cast<int>(character->c);
        return true;
    }

    return false;
}

int SemanticAnalyzer::typeCodeFor(const std::string& typeName) const {
    return symbolTable.mapTypeNameToCode(resolveTypeName(typeName));
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::describeType(TypeNode* typeNode) {
    TypeInfo info;

    if (typeNode == nullptr) {
        info.kind = "unknown";
        info.baseType = "unknown";
        return info;
    }

    if (RangeNode* range = dynamic_cast<RangeNode*>(typeNode)) {
        info.kind = "subrange";
        info.baseType = typeNameFromTypeNode(range->rangeType);

        std::string firstType = typeNameFromValueNode(range->first);
        std::string lastType = typeNameFromValueNode(range->last);
        if (info.baseType == "unknown" || info.baseType == "subrange") {
            info.baseType = firstType != "unknown" ? firstType : "integer";
        }
        if (firstType != "unknown" && lastType != "unknown" &&
            !isComparisonCompatible(firstType, lastType)) {
            addError(
                "Semantic error: subrange bounds must have compatible types, got '" +
                firstType + "' and '" + lastType + "'."
            );
        }

        if (resolveTypeName(firstType) == "real" || resolveTypeName(lastType) == "real" ||
            resolveTypeName(info.baseType) == "real") {
            addError("Semantic error: subrange type cannot be real.");
        }

        int low = 0;
        int high = 0;
        if (extractOrdinalValue(range->first, low) && extractOrdinalValue(range->last, high)) {
            info.hasBounds = true;
            info.low = low;
            info.high = high;
            if (low > high) {
                addError("Semantic error: subrange lower bound cannot be greater than upper bound.");
            }
        }
        return info;
    }

    if (ArrayTypeNode* array = dynamic_cast<ArrayTypeNode*>(typeNode)) {
        info.kind = "array";
        info.baseType = "array";
        info.indexType = typeNameFromTypeNode(array->idxType);
        info.elementType = typeNameFromTypeNode(array->elType);

        TypeInfo indexInfo = describeType(array->idxType);
        if (indexInfo.kind == "array" || indexInfo.kind == "record" ||
            resolveTypeName(indexInfo.baseType) == "real") {
            addError("Semantic error: array index type must be a simple non-real type.");
        }

        if (indexInfo.hasBounds) {
            info.hasBounds = true;
            info.low = indexInfo.low;
            info.high = indexInfo.high;
        }

        int elementRef = 0;
        auto elementInfo = typeRegistry.find(normalizeName(info.elementType));
        if (elementInfo != typeRegistry.end()) {
            elementRef = elementInfo->second.ref;
        }

        info.ref = symbolTable.addArrayType(
            typeCodeFor(info.indexType),
            typeCodeFor(info.elementType),
            elementRef,
            info.low,
            info.high
        );
        return info;
    }

    if (RecordTypeNode* record = dynamic_cast<RecordTypeNode*>(typeNode)) {
        info.kind = "record";
        info.baseType = "record";
        info.ref = symbolTable.addBlockEntry();
        int fieldOffset = 0;
        for (FieldPartNode* field : record->fieldList) {
            if (field == nullptr) {
                continue;
            }

            std::string fieldName = normalizeName(field->fieldIdent);
            if (info.fields.find(fieldName) != info.fields.end()) {
                addError("Semantic error: record field '" + field->fieldIdent + "' is already declared.");
                continue;
            }

            std::string fieldType = typeNameFromTypeNode(field->fieldType);
            TypeInfo fieldInfo = describeType(field->fieldType);
            int fieldRef = fieldInfo.ref;
            int fieldTypeCode = typeCodeFor(fieldType);
            if (fieldInfo.kind == "alias") {
                auto foundType = typeRegistry.find(normalizeName(fieldInfo.baseType));
                if (foundType != typeRegistry.end()) {
                    fieldRef = foundType->second.ref;
                    if (foundType->second.kind == "array" || foundType->second.kind == "record") {
                        fieldTypeCode = symbolTable.mapTypeNameToCode(foundType->second.kind);
                    } else {
                        fieldTypeCode = typeCodeFor(foundType->second.baseType);
                    }
                }
            } else if (fieldInfo.kind == "array" || fieldInfo.kind == "record") {
                fieldTypeCode = symbolTable.mapTypeNameToCode(fieldInfo.kind);
            }

            info.fields[fieldName] = fieldType;
            symbolTable.addRecordField(info.ref, field->fieldIdent, fieldType, fieldTypeCode, fieldRef, fieldOffset);
            fieldOffset += 1;
        }
        return info;
    }

    info.kind = "alias";
    info.baseType = normalizeName(typeNode->typeIdent);
    if (typeRegistry.find(info.baseType) == typeRegistry.end() &&
        symbolTable.lookup(info.baseType) == nullptr) {
        addError("Semantic error: type '" + typeNode->typeIdent + "' is not declared.");
        info.baseType = "unknown";
    }
    return info;
}

void SemanticAnalyzer::registerTypeDeclaration(TypeDeclNode* typeDecl) {
    if (typeDecl == nullptr) {
        return;
    }

    typeRegistry[normalizeName(typeDecl->name)] = describeType(typeDecl->simpleType);
}

std::string SemanticAnalyzer::resolveTypeName(const std::string& typeName) const {
    std::string normalizedType = normalizeName(typeName);
    if (normalizedType.empty()) {
        return "unknown";
    }

    auto found = typeRegistry.find(normalizedType);
    if (found == typeRegistry.end()) {
        return normalizedType;
    }

    const TypeInfo& info = found->second;
    if (info.kind == "simple") {
        return info.baseType.empty() ? normalizedType : info.baseType;
    }

    if (info.kind == "alias" || info.kind == "subrange") {
        if (normalizeName(info.baseType) == normalizedType) {
            return normalizedType;
        }
        return resolveTypeName(info.baseType);
    }

    if (info.kind == "array" || info.kind == "record") {
        return normalizedType;
    }

    return normalizedType;
}

bool SemanticAnalyzer::isBooleanType(const std::string& typeName) const {
    return resolveTypeName(typeName) == "boolean";
}

std::string SemanticAnalyzer::targetDisplayName(ValueNode* value) const {
    if (value == nullptr) {
        return "<unknown>";
    }

    if (VarNode* var = dynamic_cast<VarNode*>(value)) {
        return var->name;
    }

    if (ArrayAccessNode* arrayAccess = dynamic_cast<ArrayAccessNode*>(value)) {
        return arrayAccess->name + "[]";
    }

    if (RecordAccessNode* recordAccess = dynamic_cast<RecordAccessNode*>(value)) {
        return recordAccess->name + "." + recordAccess->fieldName;
    }

    return "<target>";
}

void SemanticAnalyzer::analyzeProgram(ProgramNode* program) {
    if (program == nullptr) {
        return;
    }

    program->semanticType = "program";
    program->lexicalLevel = symbolTable.currentLevel();
    declareOrReport({
        program->name,
        SymbolKind::Program,
        "program",
        symbolTable.currentLevel()
    });
    program->tabIndex = symbolTable.lookupTabIndex(program->name);

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
        std::string declaredType = typeNameFromTypeNode(varDecl->type);
        TypeInfo declaredInfo = describeType(varDecl->type);
        int ref = declaredInfo.ref;
        int typeCode = typeCodeFor(declaredType);
        if (declaredInfo.kind == "alias") {
            auto foundType = typeRegistry.find(normalizeName(declaredInfo.baseType));
            if (foundType != typeRegistry.end()) {
                ref = foundType->second.ref;
                if (foundType->second.kind == "array" || foundType->second.kind == "record") {
                    typeCode = symbolTable.mapTypeNameToCode(foundType->second.kind);
                } else {
                    typeCode = typeCodeFor(foundType->second.baseType);
                }
            }
        } else if (declaredInfo.kind == "array" || declaredInfo.kind == "record") {
            typeCode = symbolTable.mapTypeNameToCode(declaredInfo.kind);
        }

        SymbolEntry entry{
            varDecl->name,
            SymbolKind::Variable,
            declaredType,
            symbolTable.currentLevel()
        };
        entry.ref = ref;
        entry.typeCode = typeCode;
        declareOrReport(entry);
        varDecl->semanticType = declaredType;
        varDecl->tabIndex = symbolTable.lookupTabIndex(varDecl->name);
        varDecl->lexicalLevel = symbolTable.currentLevel();
        return;
    }

    if (ConstDeclNode* constDecl = dynamic_cast<ConstDeclNode*>(node)) {
        std::string valueType = inferExpressionType(constDecl->value);
        declareOrReport({
            constDecl->name,
            SymbolKind::Constant,
            valueType,
            symbolTable.currentLevel()
        });
        constDecl->semanticType = valueType;
        constDecl->tabIndex = symbolTable.lookupTabIndex(constDecl->name);
        constDecl->lexicalLevel = symbolTable.currentLevel();
        return;
    }

    if (TypeDeclNode* typeDecl = dynamic_cast<TypeDeclNode*>(node)) {
        registerTypeDeclaration(typeDecl);
        std::string typeName = typeNameFromTypeNode(typeDecl->simpleType);
        int ref = 0;
        auto foundType = typeRegistry.find(normalizeName(typeDecl->name));
        if (foundType != typeRegistry.end()) {
            ref = foundType->second.ref;
        }

        SymbolEntry entry{
            typeDecl->name,
            SymbolKind::Type,
            typeName,
            symbolTable.currentLevel()
        };
        entry.ref = ref;
        declareOrReport(entry);
        typeDecl->semanticType = typeName;
        typeDecl->tabIndex = symbolTable.lookupTabIndex(typeDecl->name);
        typeDecl->lexicalLevel = symbolTable.currentLevel();
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
        -1,
        collectParameterTypes(procDecl->parameterList),
        collectParameterNames(procDecl->parameterList)
    });
    procDecl->semanticType = "procedure";
    procDecl->tabIndex = symbolTable.lookupTabIndex(procDecl->name);
    procDecl->lexicalLevel = symbolTable.currentLevel();

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
        -1,
        collectParameterTypes(funcDecl->parameterList),
        collectParameterNames(funcDecl->parameterList)
    });
    funcDecl->semanticType = returnType;
    funcDecl->tabIndex = symbolTable.lookupTabIndex(funcDecl->name);
    funcDecl->lexicalLevel = symbolTable.currentLevel();

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

        std::string parameterType = typeNameFromTypeNode(parameter->type);
        TypeInfo parameterInfo = describeType(parameter->type);
        int ref = parameterInfo.ref;
        int typeCode = typeCodeFor(parameterType);
        if (parameterInfo.kind == "alias") {
            auto foundType = typeRegistry.find(normalizeName(parameterInfo.baseType));
            if (foundType != typeRegistry.end()) {
                ref = foundType->second.ref;
                if (foundType->second.kind == "array" || foundType->second.kind == "record") {
                    typeCode = symbolTable.mapTypeNameToCode(foundType->second.kind);
                } else {
                    typeCode = typeCodeFor(foundType->second.baseType);
                }
            }
        } else if (parameterInfo.kind == "array" || parameterInfo.kind == "record") {
            typeCode = symbolTable.mapTypeNameToCode(parameterInfo.kind);
        }

        SymbolEntry entry{
            parameter->name,
            SymbolKind::Parameter,
            parameterType,
            symbolTable.currentLevel()
        };
        entry.ref = ref;
        entry.typeCode = typeCode;
        declareOrReport(entry);
        parameter->semanticType = parameterType;
        parameter->tabIndex = symbolTable.lookupTabIndex(parameter->name);
        parameter->lexicalLevel = symbolTable.currentLevel();
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
        std::string targetType = inferExpressionType(assign->target);
        std::string valueType = inferExpressionType(assign->value);
        if (!isAssignmentCompatible(targetType, valueType)) {
            std::string targetName = targetDisplayName(assign->target);
            addError(
                "Semantic error: cannot assign value of type '" + valueType +
                "' to variable '" + targetName +
                "' of type '" + targetType + "'."
            );
        }
        if (!isValueWithinType(targetType, assign->value)) {
            addError(
                "Semantic error: value assigned to '" + targetDisplayName(assign->target) +
                "' is outside the range of type '" + targetType + "'."
            );
        }

        assign->semanticType = targetType;
        assign->lexicalLevel = symbolTable.currentLevel();
        return;
    }

    if (IfNode* ifNode = dynamic_cast<IfNode*>(node)) {
        std::string conditionType = inferExpressionType(ifNode->condition);
        if (conditionType != "unknown" && !isBooleanType(conditionType)) {
            addError(
                "Semantic error: if condition must be boolean, got '" +
                conditionType + "'."
            );
        }
        ifNode->semanticType = "boolean";
        ifNode->lexicalLevel = symbolTable.currentLevel();
        analyzeNode(ifNode->then);
        analyzeNode(ifNode->elseThen);
        return;
    }

    if (WhileNode* whileNode = dynamic_cast<WhileNode*>(node)) {
        std::string conditionType = inferExpressionType(whileNode->condition);
        if (conditionType != "unknown" && !isBooleanType(conditionType)) {
            addError(
                "Semantic error: while condition must be boolean, got '" +
                conditionType + "'."
            );
        }
        whileNode->semanticType = "boolean";
        whileNode->lexicalLevel = symbolTable.currentLevel();
        analyzeNode(whileNode->statement);
        return;
    }

    if (RepeatNode* repeatNode = dynamic_cast<RepeatNode*>(node)) {
        analyzeNode(repeatNode->statement);
        std::string conditionType = inferExpressionType(repeatNode->untilCondition);
        if (conditionType != "unknown" && !isBooleanType(conditionType)) {
            addError(
                "Semantic error: repeat-until condition must be boolean, got '" +
                conditionType + "'."
            );
        }
        repeatNode->semanticType = "boolean";
        repeatNode->lexicalLevel = symbolTable.currentLevel();
        return;
    }

    if (ForNode* forNode = dynamic_cast<ForNode*>(node)) {
        std::string loopVariableName = "";
        std::string loopVariableType = "unknown";
        std::string startType = "unknown";

        if (forNode->traversalAssign != nullptr) {
            if (forNode->traversalAssign->target != nullptr) {
                loopVariableType = inferExpressionType(forNode->traversalAssign->target);
                if (VarNode* loopVar = dynamic_cast<VarNode*>(forNode->traversalAssign->target)) {
                    loopVariableName = loopVar->name;
                    SymbolEntry* loopVariable = symbolTable.lookup(loopVariableName);
                    if (loopVariable == nullptr) {
                        checkIdentifierDeclared(loopVariableName);
                    } else {
                        loopVariableType = loopVariable->typeName;
                        if (loopVariableType != "unknown" && resolveTypeName(loopVariableType) != "integer") {
                            addError(
                                "Semantic error: for loop variable '" + loopVariableName +
                                "' must be integer, got '" + loopVariableType + "'."
                            );
                        }
                    }
                } else {
                    addError("Semantic error: for loop variable must be a simple variable.");
                }
            }

            startType = inferExpressionType(forNode->traversalAssign->value);
            if (startType != "unknown" && resolveTypeName(startType) != "integer") {
                addError(
                    "Semantic error: for loop start expression must be integer, got '" +
                    startType + "'."
                );
            }
            forNode->traversalAssign->semanticType = loopVariableType;
            forNode->traversalAssign->lexicalLevel = symbolTable.currentLevel();
        }

        std::string endType = inferExpressionType(forNode->to);
        if (endType != "unknown" && resolveTypeName(endType) != "integer") {
            addError(
                "Semantic error: for loop end expression must be integer, got '" +
                endType + "'."
            );
        }

        forNode->semanticType = "integer";
        forNode->lexicalLevel = symbolTable.currentLevel();
        analyzeNode(forNode->statement);
        return;
    }

    if (CaseNode* caseNode = dynamic_cast<CaseNode*>(node)) {
        std::string selectorType = inferExpressionType(caseNode->condition);
        for (CaseBlockNode* caseBlock : caseNode->caseBlocks) {
            if (caseBlock != nullptr) {
                std::string labelType = inferExpressionType(caseBlock->caseCondition);
                if (!isComparisonCompatible(selectorType, labelType)) {
                    addError(
                        "Semantic error: case label type '" + labelType +
                        "' is not compatible with selector type '" + selectorType + "'."
                    );
                }
            }
            analyzeNode(caseBlock);
        }
        caseNode->semanticType = selectorType;
        caseNode->lexicalLevel = symbolTable.currentLevel();
        return;
    }

    if (CaseBlockNode* caseBlock = dynamic_cast<CaseBlockNode*>(node)) {
        inferExpressionType(caseBlock->caseCondition);
        analyzeNode(caseBlock->statement);
        caseBlock->semanticType = typeNameFromValueNode(caseBlock->caseCondition);
        caseBlock->lexicalLevel = symbolTable.currentLevel();
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
            addError(
                "Semantic error: identifier '" + procCall->name +
                "' is not callable."
            );
            for (ValueNode* arg : procCall->args) {
                inferExpressionType(arg);
            }
            return;
        }

        checkCallArguments(procCall->name, procCall->args, entry);
        procCall->semanticType = entry->typeName;
        procCall->tabIndex = symbolTable.lookupTabIndex(procCall->name);
        procCall->lexicalLevel = entry->lexicalLevel;
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
        addError(
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
        addError(message);
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
    std::string normalizedName = normalizeName(name);
    return normalizedName == "read" ||
           normalizedName == "readln" ||
           normalizedName == "write" ||
           normalizedName == "writeln";
}

bool SemanticAnalyzer::isNumericType(const std::string& typeName) const {
    std::string resolvedType = resolveTypeName(typeName);
    return resolvedType == "integer" || resolvedType == "real";
}

bool SemanticAnalyzer::isSimpleAssignableType(const std::string& typeName) const {
    std::string resolvedType = resolveTypeName(typeName);
    return resolvedType == "integer" ||
           resolvedType == "real" ||
           resolvedType == "char" ||
           resolvedType == "string" ||
           resolvedType == "boolean";
}

bool SemanticAnalyzer::isAssignmentCompatible(const std::string& targetType, const std::string& valueType) const {
    std::string resolvedTarget = resolveTypeName(targetType);
    std::string resolvedValue = resolveTypeName(valueType);

    if (resolvedTarget == "unknown" || resolvedValue == "unknown") {
        return true;
    }

    if (resolvedTarget == resolvedValue) {
        return true;
    }

    if (resolvedTarget == "real" && resolvedValue == "integer") {
        return true;
    }

    if (!isSimpleAssignableType(resolvedTarget) || !isSimpleAssignableType(resolvedValue)) {
        return normalizeName(targetType) == normalizeName(valueType);
    }

    return false;
}

bool SemanticAnalyzer::isComparisonCompatible(const std::string& leftType, const std::string& rightType) const {
    std::string resolvedLeft = resolveTypeName(leftType);
    std::string resolvedRight = resolveTypeName(rightType);

    if (resolvedLeft == "unknown" || resolvedRight == "unknown") {
        return true;
    }

    if (resolvedLeft == resolvedRight) {
        return true;
    }

    return isNumericType(resolvedLeft) && isNumericType(resolvedRight);
}

bool SemanticAnalyzer::isValueWithinType(const std::string& targetType, ValueNode* valueNode) const {
    std::string currentType = normalizeName(targetType);
    std::vector<std::string> visitedTypes;

    while (!currentType.empty() &&
           std::find(visitedTypes.begin(), visitedTypes.end(), currentType) == visitedTypes.end()) {
        visitedTypes.push_back(currentType);
        auto foundType = typeRegistry.find(currentType);
        if (foundType == typeRegistry.end()) {
            return true;
        }

        const TypeInfo& info = foundType->second;
        if (info.hasBounds) {
            int value = 0;
            if (!extractOrdinalValue(valueNode, value)) {
                return true;
            }
            return value >= info.low && value <= info.high;
        }

        if (info.kind != "alias" && info.kind != "subrange") {
            return true;
        }

        currentType = normalizeName(info.baseType);
    }

    return true;
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
        addError(
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
            addError(
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
        number->semanticType = isRealLiteral(number->num) ? "real" : "integer";
        number->lexicalLevel = symbolTable.currentLevel();
        return number->semanticType;
    }

    if (StringNode* stringNode = dynamic_cast<StringNode*>(expr)) {
        stringNode->semanticType = "string";
        stringNode->lexicalLevel = symbolTable.currentLevel();
        return stringNode->semanticType;
    }

    if (CharNode* charNode = dynamic_cast<CharNode*>(expr)) {
        charNode->semanticType = "char";
        charNode->lexicalLevel = symbolTable.currentLevel();
        return charNode->semanticType;
    }

    if (VarNode* var = dynamic_cast<VarNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(var->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(var->name);
            var->semanticType = "unknown";
            return "unknown";
        }
        var->semanticType = entry->typeName;
        var->tabIndex = symbolTable.lookupTabIndex(var->name);
        var->lexicalLevel = entry->lexicalLevel;
        return var->semanticType;
    }

    if (UnaryOpNode* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        std::string op = toLowerString(unary->op);
        std::string valueType = inferExpressionType(unary->value);

        if (op == "plus" || op == "+" || op == "minus" || op == "-") {
            if (valueType != "unknown" && !isNumericType(valueType)) {
                addError(
                    "Semantic error: operator '" + unary->op +
                    "' requires numeric operand, got '" + valueType + "'."
                );
                unary->semanticType = "unknown";
                return "unknown";
            }
            unary->semanticType = valueType;
            unary->lexicalLevel = symbolTable.currentLevel();
            return valueType;
        }

        if (op == "not" || op == "notsy") {
            if (valueType != "unknown" && !isBooleanType(valueType)) {
                addError(
                    "Semantic error: operator 'not' requires boolean operand, got '" +
                    valueType + "'."
                );
                unary->semanticType = "unknown";
                return "unknown";
            }
            unary->semanticType = valueType == "unknown" ? "unknown" : "boolean";
            unary->lexicalLevel = symbolTable.currentLevel();
            return unary->semanticType;
        }

        unary->semanticType = "unknown";
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
                addError(
                    "Semantic error: operator '" + binOp->op +
                    "' requires numeric operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                binOp->semanticType = "unknown";
                return "unknown";
            }
            if (leftType == "unknown" || rightType == "unknown") {
                binOp->semanticType = "unknown";
                return "unknown";
            }
            binOp->semanticType = (resolveTypeName(leftType) == "real" ||
                                   resolveTypeName(rightType) == "real") ? "real" : "integer";
            binOp->lexicalLevel = symbolTable.currentLevel();
            return binOp->semanticType;
        }

        if (op == "rdiv" || op == "/") {
            if ((leftType != "unknown" && !isNumericType(leftType)) ||
                (rightType != "unknown" && !isNumericType(rightType))) {
                addError(
                    "Semantic error: operator '" + binOp->op +
                    "' requires numeric operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                binOp->semanticType = "unknown";
                return "unknown";
            }
            binOp->semanticType = (leftType == "unknown" || rightType == "unknown") ? "unknown" : "real";
            binOp->lexicalLevel = symbolTable.currentLevel();
            return binOp->semanticType;
        }

        if (op == "idiv" || op == "div" || op == "imod" || op == "mod") {
            if ((leftType != "unknown" && resolveTypeName(leftType) != "integer") ||
                (rightType != "unknown" && resolveTypeName(rightType) != "integer")) {
                addError(
                    "Semantic error: operator '" + binOp->op +
                    "' requires integer operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                binOp->semanticType = "unknown";
                return "unknown";
            }
            binOp->semanticType = (leftType == "unknown" || rightType == "unknown") ? "unknown" : "integer";
            binOp->lexicalLevel = symbolTable.currentLevel();
            return binOp->semanticType;
        }

        if (op == "andsy" || op == "and" || op == "orsy" || op == "or") {
            if ((leftType != "unknown" && !isBooleanType(leftType)) ||
                (rightType != "unknown" && !isBooleanType(rightType))) {
                addError(
                    "Semantic error: operator '" + binOp->op +
                    "' requires boolean operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                binOp->semanticType = "unknown";
                return "unknown";
            }
            binOp->semanticType = (leftType == "unknown" || rightType == "unknown") ? "unknown" : "boolean";
            binOp->lexicalLevel = symbolTable.currentLevel();
            return binOp->semanticType;
        }

        if (op == "eql" || op == "=" || op == "neq" || op == "<>" ||
            op == "lss" || op == "<" || op == "leq" || op == "<=" ||
            op == "gtr" || op == ">" || op == "geq" || op == ">=") {
            if (!isComparisonCompatible(leftType, rightType)) {
                addError(
                    "Semantic error: operator '" + binOp->op +
                    "' requires compatible operands, got '" + leftType +
                    "' and '" + rightType + "'."
                );
                binOp->semanticType = "unknown";
                return "unknown";
            }
            binOp->semanticType = (leftType == "unknown" || rightType == "unknown") ? "unknown" : "boolean";
            binOp->lexicalLevel = symbolTable.currentLevel();
            return binOp->semanticType;
        }

        binOp->semanticType = "unknown";
        return "unknown";
    }

    if (FuncCallNode* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(funcCall->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(funcCall->name);
            for (ValueNode* arg : funcCall->args) {
                inferExpressionType(arg);
            }
            funcCall->semanticType = "unknown";
            return "unknown";
        }

        if (entry->kind == SymbolKind::Procedure) {
            addError(
                "Semantic error: procedure '" + funcCall->name +
                "' cannot be used as a function."
            );
            checkCallArguments(funcCall->name, funcCall->args, entry);
            funcCall->semanticType = "unknown";
            return "unknown";
        }

        if (entry->kind != SymbolKind::Function) {
            addError(
                "Semantic error: identifier '" + funcCall->name +
                "' is not callable."
            );
            for (ValueNode* arg : funcCall->args) {
                inferExpressionType(arg);
            }
            funcCall->semanticType = "unknown";
            return "unknown";
        }

        checkCallArguments(funcCall->name, funcCall->args, entry);
        funcCall->semanticType = entry->typeName;
        funcCall->tabIndex = symbolTable.lookupTabIndex(funcCall->name);
        funcCall->lexicalLevel = entry->lexicalLevel;
        return funcCall->semanticType;
    }

    if (ArrayAccessNode* arrayAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(arrayAccess->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(arrayAccess->name);
            inferExpressionType(arrayAccess->idx);
            arrayAccess->semanticType = "unknown";
            return "unknown";
        }

        std::string indexType = inferExpressionType(arrayAccess->idx);
        std::string declaredType = normalizeName(entry->typeName);
        auto typeInfo = typeRegistry.find(declaredType);
        while (typeInfo != typeRegistry.end() &&
               (typeInfo->second.kind == "alias" || typeInfo->second.kind == "subrange") &&
               normalizeName(typeInfo->second.baseType) != declaredType) {
            declaredType = normalizeName(typeInfo->second.baseType);
            typeInfo = typeRegistry.find(declaredType);
        }
        if (typeInfo == typeRegistry.end() || typeInfo->second.kind != "array") {
            addError(
                "Semantic error: identifier '" + arrayAccess->name +
                "' is not an array."
            );
            arrayAccess->semanticType = "unknown";
            arrayAccess->tabIndex = symbolTable.lookupTabIndex(arrayAccess->name);
            arrayAccess->lexicalLevel = entry->lexicalLevel;
            return "unknown";
        }

        bool indexCompatible = isAssignmentCompatible(typeInfo->second.indexType, indexType);
        if (!indexCompatible) {
            addError(
                "Semantic error: array index for '" + arrayAccess->name +
                "' expects '" + typeInfo->second.indexType +
                "', got '" + indexType + "'."
            );
        }
        if (indexCompatible && typeInfo->second.hasBounds) {
            int indexValue = 0;
            if (extractOrdinalValue(arrayAccess->idx, indexValue) &&
                (indexValue < typeInfo->second.low || indexValue > typeInfo->second.high)) {
                addError(
                    "Semantic error: array index for '" + arrayAccess->name +
                    "' is outside declared bounds."
                );
            }
        }

        arrayAccess->semanticType = typeInfo->second.elementType;
        arrayAccess->tabIndex = symbolTable.lookupTabIndex(arrayAccess->name);
        arrayAccess->lexicalLevel = entry->lexicalLevel;
        return arrayAccess->semanticType;
    }

    if (RecordAccessNode* recordAccess = dynamic_cast<RecordAccessNode*>(expr)) {
        SymbolEntry* entry = symbolTable.lookup(recordAccess->name);
        if (entry == nullptr) {
            checkIdentifierDeclared(recordAccess->name);
            recordAccess->semanticType = "unknown";
            return "unknown";
        }

        std::string declaredType = normalizeName(entry->typeName);
        auto typeInfo = typeRegistry.find(declaredType);
        while (typeInfo != typeRegistry.end() &&
               (typeInfo->second.kind == "alias" || typeInfo->second.kind == "subrange") &&
               normalizeName(typeInfo->second.baseType) != declaredType) {
            declaredType = normalizeName(typeInfo->second.baseType);
            typeInfo = typeRegistry.find(declaredType);
        }
        if (typeInfo == typeRegistry.end() || typeInfo->second.kind != "record") {
            addError(
                "Semantic error: identifier '" + recordAccess->name +
                "' is not a record."
            );
            recordAccess->semanticType = "unknown";
            recordAccess->tabIndex = symbolTable.lookupTabIndex(recordAccess->name);
            recordAccess->lexicalLevel = entry->lexicalLevel;
            return "unknown";
        }

        auto field = typeInfo->second.fields.find(normalizeName(recordAccess->fieldName));
        if (field == typeInfo->second.fields.end()) {
            addError(
                "Semantic error: record '" + recordAccess->name +
                "' has no field '" + recordAccess->fieldName + "'."
            );
            recordAccess->semanticType = "unknown";
            recordAccess->tabIndex = symbolTable.lookupTabIndex(recordAccess->name);
            recordAccess->lexicalLevel = entry->lexicalLevel;
            return "unknown";
        }

        recordAccess->semanticType = field->second;
        recordAccess->tabIndex = symbolTable.lookupTabIndex(recordAccess->name);
        recordAccess->lexicalLevel = entry->lexicalLevel;
        return recordAccess->semanticType;
    }

    expr->semanticType = "unknown";
    return "unknown";
}

std::string SemanticAnalyzer::typeNameFromTypeNode(TypeNode* typeNode) const {
    if (typeNode == nullptr) {
        return "unknown";
    }

    if (RangeNode* range = dynamic_cast<RangeNode*>(typeNode)) {
        std::string rangeType = typeNameFromTypeNode(range->rangeType);
        if (rangeType == "unknown" || rangeType == "subrange") {
            std::string firstType = typeNameFromValueNode(range->first);
            return firstType == "unknown" ? "integer" : firstType;
        }
        return rangeType;
    }

    if (dynamic_cast<ArrayTypeNode*>(typeNode) != nullptr) {
        return "array";
    }

    if (dynamic_cast<RecordTypeNode*>(typeNode) != nullptr) {
        return "record";
    }

    return normalizeName(typeNode->typeIdent);
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

    if (BinOpNode* binOp = dynamic_cast<BinOpNode*>(valueNode)) {
        std::string op = toLowerString(binOp->op);
        if (op == "eql" || op == "=" || op == "neq" || op == "<>" ||
            op == "lss" || op == "<" || op == "leq" || op == "<=" ||
            op == "gtr" || op == ">" || op == "geq" || op == ">=" ||
            op == "andsy" || op == "and" || op == "orsy" || op == "or") {
            return "boolean";
        }
    }

    return "unknown";
}
