#include "token.hpp"

const std::string Token::tokenTypeToString() const {
    switch(type) {
        case stringcon:
            return "stringcon (" + value +")";
        case charcon:
            return "charcon (" + value +")";
        case ListToken::plus:
            return "plus";
        case ListToken::minus:
            return "minus";
        case times:
            return "times";
        case rdiv:
            return "rdiv";
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
        default:
            return "how bro";
    }
}