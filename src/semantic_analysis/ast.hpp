#pragma once
#include "../parser/parser.hpp"
#include "type.hpp"
#include <iostream>
using namespace std;

enum ASTNodeType {
    PROGRAM_NODE,
    DECLARATIONS,
    VAR_DECL_NODE,
    TYPE_DECL_NODE,
    CONST_DECL_NODE,
    PROC_DECL_NODE,
    FUNC_DECL_NODE,
    PARAMETER_NODE,
    TYPE_NODE,
    FIELD_PART_NODE,
    BLOCK,
    ASSIGN_NODE,
    PROC_CALL_NODE,
    FUNC_CALL_NODE,
    NUMBER_NODE,
    STRING_NODE,
    CHAR_NODE,
    VAR_NODE,
    UNOP_NODE,
    BINOP_NODE,
    IF_NODE,
    WHILE_NODE,
    CASE_NODE,
    CASE_BLOCK_NODE,
    REPEAT_NODE,
    FOR_NODE,
    ARRAY_ACCESS_NODE,
    RECORD_ACCESS_NODE,
};

class ASTNode {
    protected:
        ASTNodeType nodeType;
        vector<ASTNode*> children;
    public:
        ASTNode(ASTNodeType nodeType) : nodeType(nodeType) {}
        vector<ASTNode*> getChildren() { return children; }
        void addChild(ASTNode* node) { children.push_back(node); }
        virtual void visit() = 0;
        virtual void print() const = 0;
};

class ProgramNode : public ASTNode {
    public:
        string name;
        
    
        ProgramNode(string name) : ASTNode(PROGRAM_NODE), name(name) {}
        void visit() override;
        void print() const override { cout<<"ProgramNode(name: '"<<name<<"')"; }
};



class ValueNode : public ASTNode {
    public:
        ValueNode(ASTNodeType nodeType) : ASTNode(nodeType) {}
};

class NumberNode : public ValueNode {
    public:
        string num;
    
        NumberNode(string num) : ValueNode(NUMBER_NODE), num(num) {}
        void visit() override;
        void print() const override { cout<<"Number("<<num<<")"; }
};

class CharNode : public ValueNode {
    public:
        char c;
    
        CharNode(char c) : ValueNode(CHAR_NODE), c(c) {}
        void visit() override;
        void print() const override { cout<<"Char('"<<c<<"')"; }
};

class StringNode : public ValueNode {
    public:
        string str;
    
        StringNode(string str) : ValueNode(STRING_NODE), str(str) {}
        void visit() override;
        void print() const override { cout<<"String('"<<str<<"')"; }
};

class VarNode : public ValueNode {
    public:
        string name;
    
        VarNode(string name) : ValueNode(VAR_NODE), name(name) {}
        void visit() override;
        void print() const override { cout<<"Var('"<<name<<"')"; }
};

class UnaryOpNode : public ValueNode {
    public:
        string op;
        ValueNode* value;
    
        UnaryOpNode(string op, ValueNode* value) : ValueNode(UNOP_NODE), op(op), value(value) {}
        void visit() override;
        void print() const override { cout<<"ArrayType(op: "<<op<<", value: ";
                                      value->print();
                                      cout<<")";}
};

class BinOpNode : public ValueNode {
    public:
        string op;
        ValueNode* left;
        ValueNode* right;
    
        BinOpNode(string op, ValueNode* left, ValueNode* right) : ValueNode(BINOP_NODE), op(op), left(left), right(right) {}
        void visit() override;
        void print() const override { cout<<"ArrayType(op: "<<op<<", left: ";
                                      left->print();
                                      cout<<", right: ";
                                      right->print();
                                      cout<<")";}
};

class TypeNode : public ASTNode {
    public:
        string typeIdent;
    
        TypeNode(string typeIdent) : ASTNode(TYPE_NODE), typeIdent(typeIdent) {}
        virtual void visit() override;
        virtual void print() const override { cout<<"Type(type: '"<<typeIdent<<"')"; }
};

class RangeNode : public TypeNode {
    public:
        ValueNode* first;
        ValueNode* last;
        TypeNode* rangeType;
    
        RangeNode(ValueNode* first, ValueNode* last, TypeNode* type) : TypeNode("subrange"), first(first), last(last), rangeType(type) {}
        void visit() override;
        void print() const override { cout<<"Range(first: ";
                                      first->print();
                                      cout<<", last: ";
                                      last ->print();
                                      cout<<", type: ";
                                      rangeType->print();
                                      cout<<")";}
};

class ArrayTypeNode : public TypeNode {
    public:
        TypeNode* idxType;
        TypeNode* elType;
    
        ArrayTypeNode(TypeNode* idxType, TypeNode* elType) : TypeNode("array"), idxType(idxType), elType(elType) {}
        void visit() override;
        void print() const override { cout<<"ArrayType(idxType: ";
                                      idxType->print();
                                      cout<<", elType: ";
                                      elType->print();
                                      cout<<")";}
};

class RecordTypeNode : public TypeNode {
    public:
        vector<FieldPartNode*> fieldList;
    
        RecordTypeNode() : TypeNode("record") {}
        void visit() override;
        void print() const override {
            cout<<"RecordType(fieldList: [";
            for(int i = 0; i < fieldList.size(); i++) {
                fieldList[i]->print();
                if (i != fieldList.size() - 1) {
                    cout<<", ";
                }
            }
            cout<<"])";
        }
};


class FieldPartNode : public ASTNode {
    public:
        string fieldIdent;
        TypeNode* fieldType;
    
        FieldPartNode(string fieldIdent, TypeNode* fieldType) : ASTNode(FIELD_PART_NODE), fieldIdent(fieldIdent), fieldType(fieldType) {}
        void visit() override;
        void print() const override { cout<<"FieldPart(fieldIdent: "<<fieldIdent<<", type: ";
                                              fieldType->print();
                                              cout<<")";}
};


class VarDeclNode : public ASTNode {
    public:
        string name;
        TypeNode* type;
    
        VarDeclNode(string name, TypeNode* type) : ASTNode(VAR_DECL_NODE), name(name), type(type) {}
        void visit() override;
        void print() const override { cout<<"VarDecl(name: "<<name<<", type: ";
                                      type->print();
                                      cout<<")";}
};

class TypeDeclNode : public ASTNode {
    public:
        string name;
        TypeNode* simpleType;
    
        TypeDeclNode(string name, TypeNode* simpleType) : ASTNode(TYPE_DECL_NODE), name(name), simpleType(simpleType) {}
        void visit() override;
        void print() const override { cout<<"TypeDecl(name: "<<name<<", simpleType: ";
                                      simpleType->print();
                                      cout<<")";}
};

class ConstDeclNode : public ASTNode {
    public:
        string name;
        ValueNode* value;
    
        ConstDeclNode(string name, ValueNode* value) : ASTNode(CONST_DECL_NODE), name(name), value(value) {}
        void visit() override;
        void print() const override { cout<<"ConstDecl(name: "<<name<<", value: ";
                                      value->print();
                                      cout<<")";}
};

class ParameterNode : public ASTNode {
    public:
        string name;
        TypeNode* type;
    
        ParameterNode(string name, TypeNode* type) : ASTNode(PARAMETER_NODE), name(name), type(type) {}
        void visit() override;
        void print() const override { cout<<"Parameter(name: "<<name<<", type: ";
                                      type->print();
                                      cout<<")";}
};

class FuncDeclNode : public ASTNode {
    public:
        string name;
        vector<ParameterNode*> parameterList;
        TypeNode* returnType;
    
        FuncDeclNode(string name, TypeNode* returnType) : ASTNode(FUNC_DECL_NODE), name(name), returnType(returnType) {}
        void visit() override;
        void print() const override { 
            cout<<"FunctionDecl(name: "<<name<<", returnType: "<<returnType<<")";}
};

class ProcDeclNode : public ASTNode {
    public:
        string name;
        vector<ParameterNode*> parameterList;
    
        ProcDeclNode(string name, TypeNode* returnType) : ASTNode(PROC_DECL_NODE), name(name){}
        void visit() override;
        void print() const override { 
            cout<<"ProcedureDecl(name: "<<name<<")";}
};


class AssignNode : public ASTNode {
    public:
        VarNode* target;
        ValueNode* value;
    
        AssignNode(VarNode* target, ValueNode* value) : ASTNode(ASSIGN_NODE), target(target), value(value) {}
        void visit() override;
        void print() const override { cout<<"Assign(target: ";
                                      target->print();
                                      cout<<", value: ";
                                      value->print();
                                      cout<<")";}
};

class IfNode : public ASTNode {
    public:
        ValueNode* condition;
        ASTNode* then;
        ASTNode* elseThen;
    
        IfNode(ValueNode* condition, ASTNode* then, ASTNode* elseThen) : ASTNode(IF_NODE), condition(condition), then(then), elseThen(elseThen) {}
        void visit() override;
};

class CaseBlockNode : public ASTNode {
    public:
        ValueNode* caseCondition;
        ASTNode* statement;
    
        CaseBlockNode(ValueNode* caseCondition, ASTNode* statement) : ASTNode(CASE_BLOCK_NODE), caseCondition(caseCondition), statement(statement) {}
        void visit() override;
};

class CaseNode : public ASTNode {
    public:
        ValueNode* condition;
        vector<CaseBlockNode*> caseBlocks;
    
        CaseNode(ValueNode* condition) : ASTNode(CASE_NODE), condition(condition) {}
        void visit() override;
};

class WhileNode : public ASTNode {
    public:
        ValueNode* condition;
        ASTNode* statement;
    
        WhileNode(ValueNode* condition, ASTNode* statement) : ASTNode(WHILE_NODE), condition(condition), statement(statement) {}
        void visit() override;
};

class RepeatNode : public ASTNode {
    public:
        ValueNode* untilCondition;
        ASTNode* statement;
    
        RepeatNode(ValueNode* condition, ASTNode* statement) : ASTNode(REPEAT_NODE), untilCondition(condition), statement(statement) {}
        void visit() override;
};

class ForNode : public ASTNode {
    public:
        AssignNode* traversalAssign;
        ValueNode* to;
        ASTNode* statement;
    
        ForNode(AssignNode* traversalAssign, ValueNode* to, ASTNode* statement) : ASTNode(FOR_NODE), traversalAssign(traversalAssign), to(to), statement(statement) {}
        void visit() override;
};

class ProcCallNode : public ASTNode {
    public:
        string name;
        vector<ValueNode*> args;

        ProcCallNode(string name) : ASTNode(PROC_CALL_NODE), name(name) {}
        void visit() override;
};

class FuncCallNode : public ValueNode {
    public:
        string name;
        vector<ValueNode*> args;

        FuncCallNode(string name) : ValueNode(FUNC_CALL_NODE), name(name) {}
        void visit() override;
};

class ArrayAccessNode : public ValueNode {
    public:
        string name;
        ValueNode* idx;

        ArrayAccessNode(string name, ValueNode* idx) : ValueNode(ARRAY_ACCESS_NODE), name(name), idx(idx) {}
        void visit() override;
};

class RecordAccessNode : public ValueNode {
    public:
        string name;
        string fieldName;
    
        RecordAccessNode(string name, string fieldName) : ValueNode(RECORD_ACCESS_NODE), name(name), fieldName(fieldName) {}
        void visit() override;
};


ASTNode* buildAST(TreeParser* root);

void printAST(ASTNode* tree);