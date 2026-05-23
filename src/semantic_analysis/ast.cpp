#include "ast.hpp"
#include "semantic_analyzer.hpp"
#include <functional>

void ProgramNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitProgram(this); }
void NumberNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitNumber(this); }
void CharNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitChar(this); }
void StringNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitString(this); }
void VarNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitVar(this); }
void UnaryOpNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitUnaryOp(this); }
void BinOpNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitBinOp(this); }
void TypeNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitType(this); }
void RangeNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitRange(this); }
void ArrayTypeNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitArrayType(this); }
void EnumeratedTypeNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitEnumeratedType(this); }
void RecordTypeNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitRecordType(this); }
void FieldPartNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitFieldPart(this); }
void VarDeclNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitVarDecl(this); }
void TypeDeclNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitTypeDecl(this); }
void ConstDeclNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitConstDecl(this); }
void ParameterNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitParameter(this); }
void FuncDeclNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitFuncDecl(this); }
void ProcDeclNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitProcDecl(this); }
void AssignNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitAssign(this); }
void IfNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitIf(this); }
void CaseBlockNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitCaseBlock(this); }
void CaseNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitCase(this); }
void WhileNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitWhile(this); }
void RepeatNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitRepeat(this); }
void ForNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitFor(this); }
void ProcCallNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitProcCall(this); }
void FuncCallNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitFuncCall(this); }
void ArrayAccessNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitArrayAccess(this); }
void RecordAccessNode::visit(SemanticAnalyzer& analyzer) { analyzer.visitRecordAccess(this); }

static string extractIdent(const string& s) {
    size_t lp = s.find('(');
    size_t rp = s.rfind(')');
    if (lp == string::npos || rp == string::npos) return s;
    return s.substr(lp + 1, rp - lp - 1);
}

static TreeParser* findChild(TreeParser* node, const string& name) {
    for (auto* c : node->children)
        if (c->data == name) return c;
    return nullptr;
}

static ValueNode* buildExpression(TreeParser* node);
static ValueNode* buildSimpleExpression(TreeParser* node);
static ValueNode* buildTerm(TreeParser* node);
static ValueNode* buildFactor(TreeParser* node);
static ValueNode* buildVariable(TreeParser* node);
static TypeNode*  buildType(TreeParser* node);
static ASTNode*   buildStatement(TreeParser* node);
static ASTNode*   buildStatementList(TreeParser* node);
static ASTNode*   buildDeclarationPart(TreeParser* node);

static ValueNode* buildIndexValue(TreeParser* node) {
    if (!node || node->children.empty()) return nullptr;

    string d = node->children[0]->data;
    if (d.size() >= 6 && d.substr(0, 6) == "intcon")
        return new NumberNode(extractIdent(d));
    if (d.size() >= 6 && d.substr(0, 6) == "charco") {
        size_t lp = d.find('(');
        char c = (lp != string::npos && lp + 1 < d.size()) ? d[lp + 1] : '?';
        return new CharNode(c);
    }
    if (d.size() >= 5 && d.substr(0, 5) == "ident")
        return new VarNode(extractIdent(d));

    return new VarNode(d);
}

static ValueNode* buildVariable(TreeParser* node) {
    if (!node || node->children.empty()) return new VarNode("");

    string baseName = extractIdent(node->children[0]->data);
    if (node->children.size() == 1) {
        return new VarNode(baseName);
    }

    TreeParser* component = node->children[1];
    if (!component || component->children.empty()) {
        return new VarNode(baseName);
    }

    if (component->children[0]->data == "lbrack" && component->children.size() > 1) {
        return new ArrayAccessNode(baseName, buildIndexValue(component->children[1]));
    }

    if (component->children[0]->data == "period" && component->children.size() > 1) {
        return new RecordAccessNode(baseName, extractIdent(component->children[1]->data));
    }

    return new VarNode(baseName);
}

static ValueNode* buildFactor(TreeParser* node) {
    TreeParser* first = node->children[0];

    if (first->data == "notsy")
        return new UnaryOpNode("not", buildFactor(node->children[1]));

    if (first->data == "lparent")
        return buildExpression(node->children[1]);

    if (first->data == "<procedure/function-call>") {
        string name = extractIdent(first->children[0]->data);
        FuncCallNode* callNode = new FuncCallNode(name);
        TreeParser* paramList = findChild(first, "<parameter-list>");
        if (paramList)
            for (auto* c : paramList->children)
                if (c->data == "<expression>")
                    callNode->args.push_back(buildExpression(c));
        return callNode;
    }

    if (first->data == "<variable>")
        return buildVariable(first);

    string d = first->data;

    if (d.size() >= 5 && d.substr(0, 5) == "ident")
        return new VarNode(extractIdent(d));
    if (d.size() >= 6 && d.substr(0, 6) == "intcon")
        return new NumberNode(extractIdent(d));
    if (d.size() >= 7 && d.substr(0, 7) == "realcon")
        return new NumberNode(extractIdent(d));
    if (d.size() >= 6 && d.substr(0, 6) == "charco") {
        size_t lp = d.find('(');
        char c = (lp != string::npos && lp + 1 < d.size()) ? d[lp + 1] : '?';
        return new CharNode(c);
    }
    if (d.size() >= 9 && d.substr(0, 9) == "stringcon")
        return new StringNode(extractIdent(d));

    return new VarNode(d);
}

static ValueNode* buildTerm(TreeParser* node) {
    auto& ch = node->children;
    ValueNode* result = buildFactor(ch[0]);
    for (int i = 1; i < (int)ch.size(); i += 2) {
        string op    = ch[i]->children[0]->data;
        ValueNode* r = buildFactor(ch[i + 1]);
        result = new BinOpNode(op, result, r);
    }
    return result;
}

static ValueNode* buildSimpleExpression(TreeParser* node) {
    auto& ch = node->children;
    int i = 0;

    string unary = "";
    if (ch[i]->data == "plus" || ch[i]->data == "minus") {
        unary = ch[i]->data;
        i++;
    }

    ValueNode* result = buildTerm(ch[i++]);

    if (!unary.empty())
        result = new UnaryOpNode(unary, result);

    while (i < (int)ch.size()) {
        string op    = ch[i]->children[0]->data;
        ValueNode* r = buildTerm(ch[i + 1]);
        result = new BinOpNode(op, result, r);
        i += 2;
    }
    return result;
}

static ValueNode* buildExpression(TreeParser* node) {
    auto& ch = node->children;
    ValueNode* left = buildSimpleExpression(ch[0]);
    if (ch.size() == 3) {
        string op    = ch[1]->children[0]->data;
        ValueNode* r = buildSimpleExpression(ch[2]);
        return new BinOpNode(op, left, r);
    }
    return left;
}

static TypeNode* buildType(TreeParser* node) {
    TreeParser* inner = node->children[0];

    if (inner->data == "<array-type>") {
        auto& ch = inner->children;
        TypeNode* idxType = nullptr;
        if (ch[2]->data == "<range>") {
            ValueNode* first = buildExpression(ch[2]->children[0]);
            ValueNode* last  = buildExpression(ch[2]->children[3]);
            idxType = new RangeNode(first, last, new TypeNode("integer"));
        } else {
            idxType = new TypeNode(extractIdent(ch[2]->data));
        }
        TypeNode* elType = buildType(ch[5]);
        return new ArrayTypeNode(idxType, elType);
    }

    if (inner->data == "<record-type>") {
        RecordTypeNode* recNode = new RecordTypeNode();
        TreeParser* fieldList = findChild(inner, "<field-list>");
        if (fieldList)
            for (auto* c : fieldList->children)
                if (c->data == "<field-part>") {
                    TreeParser* idList = c->children[0];
                    TypeNode*   fType  = buildType(c->children[2]);
                    for (auto* idChild : idList->children)
                        if (idChild->data.size() >= 5 &&
                            idChild->data.substr(0, 5) == "ident")
                            recNode->fieldList.push_back(
                                new FieldPartNode(extractIdent(idChild->data), fType));
                }
        return recNode;
    }

    if (inner->data == "<enumerated>") {
        EnumeratedTypeNode* enumNode = new EnumeratedTypeNode();
        for (TreeParser* child : inner->children) {
            if (child->data.size() >= 5 && child->data.substr(0, 5) == "ident") {
                enumNode->members.push_back(extractIdent(child->data));
            }
        }
        return enumNode;
    }

    if (inner->data == "<range>") {
        ValueNode* first = buildExpression(inner->children[0]);
        ValueNode* last  = buildExpression(inner->children[3]);
        return new RangeNode(first, last, new TypeNode("subrange"));
    }

    return new TypeNode(extractIdent(inner->data));
}

static ValueNode* buildConstant(TreeParser* node) {
    auto& ch = node->children;
    int i = 0;

    string unary = "";
    if (ch[i]->data == "plus" || ch[i]->data == "minus") {
        unary = ch[i]->data;
        i++;
    }

    string d = ch[i]->data;
    ValueNode* result = nullptr;

    if (d.size() >= 6 && d.substr(0, 6) == "charco") {
        size_t lp = d.find('(');
        result = new CharNode((lp != string::npos) ? d[lp + 1] : '?');
    } else if (d.size() >= 9 && d.substr(0, 9) == "stringcon") {
        result = new StringNode(extractIdent(d));
    } else if (d.size() >= 6 && d.substr(0, 6) == "intcon") {
        result = new NumberNode(extractIdent(d));
    } else if (d.size() >= 7 && d.substr(0, 7) == "realcon") {
        result = new NumberNode(extractIdent(d));
    } else {
        result = new VarNode(extractIdent(d));
    }

    if (!unary.empty())
        result = new UnaryOpNode(unary, result);

    return result;
}

static vector<ParameterNode*> buildFormalParams(TreeParser* fpl) {
    vector<ParameterNode*> params;
    for (auto* c : fpl->children) {
        if (c->data != "<parameter-group>") continue;
        auto& ch = c->children;
        int j = 0;
        if (ch[j]->data == "varsy") j++;
        TreeParser* idList = ch[j++];
        j++;
        TypeNode* pType = nullptr;
        if (ch[j]->data == "<array-type>") {
            TreeParser dummy("<type>");
            dummy.children.push_back(ch[j]);
            pType = buildType(&dummy);
        } else {
            pType = new TypeNode(extractIdent(ch[j]->data));
        }
        for (auto* idChild : idList->children)
            if (idChild->data.size() >= 5 && idChild->data.substr(0, 5) == "ident")
                params.push_back(new ParameterNode(extractIdent(idChild->data), pType));
    }
    return params;
}

static ASTNode* buildStatement(TreeParser* node) {
    if (node->children.empty()) return nullptr;

    TreeParser* inner = node->children[0];

    if (inner->data == "<assignment-statement>") {
        ValueNode* target = buildVariable(inner->children[0]);
        ValueNode* val = buildExpression(inner->children[2]);
        return new AssignNode(target, val);
    }

    if (inner->data == "<procedure/function-call>") {
        string name = extractIdent(inner->children[0]->data);
        ProcCallNode* callNode = new ProcCallNode(name);
        TreeParser* paramList = findChild(inner, "<parameter-list>");
        if (paramList)
            for (auto* c : paramList->children)
                if (c->data == "<expression>")
                    callNode->args.push_back(buildExpression(c));
        return callNode;
    }

    if (inner->data == "<if-statement>") {
        auto& ch      = inner->children;
        ValueNode* cond  = buildExpression(ch[1]);
        ASTNode*   thenSt = buildStatement(ch[3]);
        ASTNode*   elseSt = (ch.size() > 4) ? buildStatement(ch[5]) : nullptr;
        return new IfNode(cond, thenSt, elseSt);
    }

    if (inner->data == "<while-statement>") {
        ValueNode* cond = buildExpression(inner->children[1]);
        ASTNode*   stmt = buildStatementList(inner->children[3]->children[1]);
        return new WhileNode(cond, stmt);
    }

    if (inner->data == "<repeat-statement>") {
        ASTNode*   stmts = buildStatementList(inner->children[1]);
        ValueNode* cond  = buildExpression(inner->children[3]);
        return new RepeatNode(cond, stmts);
    }

    if (inner->data == "<for-statement>") {
        auto& ch = inner->children;
        string varName     = extractIdent(ch[1]->data);
        ValueNode* from    = buildExpression(ch[3]);
        AssignNode* assign = new AssignNode(new VarNode(varName), from);
        ValueNode* toVal   = buildExpression(ch[5]);
        ASTNode*   stmt    = buildStatementList(ch[7]->children[1]);
        return new ForNode(assign, toVal, stmt);
    }

    if (inner->data == "<case-statement>") {
        ValueNode* cond    = buildExpression(inner->children[1]);
        CaseNode*  caseNode = new CaseNode(cond);

        function<void(TreeParser*)> collectCases = [&](TreeParser* cb) {
            if (!cb || cb->data != "<case-block>") return;
            auto& ch = cb->children;
            int i = 0;

            vector<ValueNode*> consts;
            while (i < (int)ch.size() && ch[i]->data != "colon") {
                if (ch[i]->data == "<constant>")
                    consts.push_back(buildConstant(ch[i]));
                i++;
            }
            i++;

            ASTNode* stmt = buildStatement(ch[i++]);

            for (auto* cv : consts)
                caseNode->caseBlocks.push_back(new CaseBlockNode(cv, stmt));

            while (i < (int)ch.size()) {
                if (ch[i]->data == "<case-block>") collectCases(ch[i]);
                i++;
            }
        };

        collectCases(findChild(inner, "<case-block>"));
        return caseNode;
    }

    if (inner->data == "<compound-statement>")
        return buildStatementList(inner->children[1]);

    return nullptr;
}

static ASTNode* buildStatementList(TreeParser* node) {
    ProcCallNode* block = new ProcCallNode("__block__");
    for (auto* c : node->children)
        if (c->data == "<statement>") {
            ASTNode* stmt = buildStatement(c);
            if (stmt) block->addChild(stmt);
        }
    return block;
}

static ASTNode* buildDeclarationPart(TreeParser* node) {
    ProcCallNode* decls = new ProcCallNode("__declarations__");

    for (auto* c : node->children) {

        if (c->data == "<const-declaration>") {
            auto& ch = c->children;
            int i = 1;
            while (i < (int)ch.size()) {
                if (ch[i]->data.size() >= 5 && ch[i]->data.substr(0,5) == "ident") {
                    string name    = extractIdent(ch[i]->data); i++;
                    i++;
                    ValueNode* val = buildConstant(ch[i]); i++;
                    i++;
                    decls->addChild(new ConstDeclNode(name, val));
                } else { i++; }
            }
        }

        if (c->data == "<type-declaration>") {
            auto& ch = c->children;
            int i = 1;
            while (i < (int)ch.size()) {
                if (ch[i]->data.size() >= 5 && ch[i]->data.substr(0,5) == "ident") {
                    string name  = extractIdent(ch[i]->data); i++;
                    i++;
                    TypeNode* t  = buildType(ch[i]); i++;
                    i++;
                    decls->addChild(new TypeDeclNode(name, t));
                } else { i++; }
            }
        }

        if (c->data == "<var-declaration>") {
            auto& ch = c->children;
            int i = 1;
            while (i < (int)ch.size()) {
                if (ch[i]->data == "<identifier-list>") {
                    TreeParser* idList = ch[i++];
                    i++;
                    TypeNode* t = buildType(ch[i++]);
                    i++;
                    for (auto* idChild : idList->children)
                        if (idChild->data.size() >= 5 &&
                            idChild->data.substr(0, 5) == "ident")
                            decls->addChild(
                                new VarDeclNode(extractIdent(idChild->data), t));
                } else { i++; }
            }
        }

        if (c->data == "<subprogram-declaration>") {
            TreeParser* subp = c->children[0];

            if (subp->data == "<procedure-declaration>") {
                auto& ch = subp->children;
                string name = extractIdent(ch[1]->data);
                ProcDeclNode* proc = new ProcDeclNode(name, nullptr);
                int i = 2;
                if (ch[i]->data == "<formal-parameter-list>") {
                    proc->parameterList = buildFormalParams(ch[i++]);
                }
                i++;
                TreeParser* blk = ch[i];
                proc->addChild(buildDeclarationPart(blk->children[0]));
                proc->addChild(buildStatementList(blk->children[1]->children[1]));
                decls->addChild(proc);
            }

            if (subp->data == "<function-declaration>") {
                auto& ch = subp->children;
                string name = extractIdent(ch[1]->data);
                int i = 2;
                vector<ParameterNode*> params;
                if (ch[i]->data == "<formal-parameter-list>") {
                    params = buildFormalParams(ch[i++]);
                }
                i++;
                TypeNode* retType = new TypeNode(extractIdent(ch[i++]->data));
                i++;
                FuncDeclNode* func = new FuncDeclNode(name, retType);
                func->parameterList = params;
                TreeParser* blk = ch[i];
                func->addChild(buildDeclarationPart(blk->children[0]));
                func->addChild(buildStatementList(blk->children[1]->children[1]));
                decls->addChild(func);
            }
        }
    }

    return decls;
}

ASTNode* buildAST(TreeParser* root) {
    if (!root || root->data != "<program>") return nullptr;

    string progName = extractIdent(root->children[0]->children[1]->data);
    ProgramNode* program = new ProgramNode(progName);

    program->addChild(buildDeclarationPart(root->children[1]));
    program->addChild(buildStatementList(root->children[2]->children[1]));

    return program;
}
