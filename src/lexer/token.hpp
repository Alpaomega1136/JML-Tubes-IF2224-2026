#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum ListToken {
    intcon,
    realcon,
    charcon,
    stringcon,

    plus,
    minus,
    times,
    idiv,
    rdiv,
    imod,

    notsy,
    andsy,
    orsy,

    eql,
    neq,
    gtr,
    geq,
    lss,
    leq,

    lparent,
    rparent,
    lbrack,
    rbrack,
    comma,
    semicolon,
    period,
    colon,
    becomes,

    constsy,
    typesy,
    varsy,
    functionsy,
    proceduresy,
    arraysy,
    recordsy,
    programsy,

    beginsy,
    endsy,
    ifsy,
    thensy,
    elsesy,
    casesy,
    ofsy,
    whilesy,
    dosy,
    repeatsy,
    untilsy,
    forsy,
    tosy,
    downtosy,

    ident,
    comment,

    unknown
};

class Token {
    public:
        ListToken type;
        std::string value;
        Token(ListToken type, std::string val) : type(type), value(val) {}
        const std::string tokenTypeToString() const;
};

#endif
