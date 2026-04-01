#include "token.hpp"

const std::string Token::tokenTypeToString() const {
    switch(type) {
        case intcon:
            return "intcon (" + value + ")";
        case realcon:
            return "realcon (" + value + ")";
        case stringcon:
            return "stringcon (" + value + ")";
        case charcon:
            return "charcon (" + value + ")";
        case notsy:
            return "notsy";
        case ListToken::plus:
            return "plus";
        case ListToken::minus:
            return "minus";
        case times:
            return "times";
        case idiv:
            return "div";
        case rdiv:
            return "rdiv";
        case imod:
            return "imod";
        case andsy:
            return "andsy";
        case orsy:
            return "orsy";
        case eql:
            return "eql";
        case neq:
            return "neq";
        case gtr:
            return "gtr";
        case geq:
            return "geq";
        case lss:
            return "lss"; 
        case leq:
            return "leq";
        case lparent:
            return "lparent";
        case rparent:
            return "rparent";
        case lbrack:
            return "lbrack";
        case rbrack:
            return "rbrack";
        case comma:
            return "comma";
        case semicolon:
            return "semicolon";
        case period:
            return "period";
        case colon:
            return "colon";
        case becomes:
            return "becomes";
        case constsy:
            return "constsy";
        case typesy:
            return "typesy";
        case varsy:
            return "varsy";
        case functionsy:
            return "functionsy";
        case proceduresy:
            return "proceduresy";
        case arraysy:
            return "arraysy";
        case recordsy:
            return "recordsy";
        case programsy:
            return "programsy";
        case ident:
            return "ident (" + value + ")";
        case beginsy:
            return "beginsy";
        case ifsy:
            return "ifsy";
        case casesy:
            return "casesy";
        case repeatsy:
            return "repeatsy";
        case whilesy:
            return "whilesy";
        case forsy:
            return "forsy";
        case endsy:
            return "endsy";
        case elsesy:
            return "elsesy";
        case untilsy:
            return "untilsy";
        case ofsy:
            return "ofsy";
        case dosy:
            return "dosy";
        case tosy:
            return "tosy";
        case downtosy:
            return "downtosy";
        case thensy:
            return "thensy";
        case comment:
            return "comment (" + value + ")";
        case unknown:
            return "unknown (" + value + ")";
        default:
            return "how bro";
    }
}