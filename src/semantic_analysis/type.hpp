#pragma once
#include <iostream>
using namespace std;

enum SimpleType {
    REAL,
    INTEGER,
    CHAR,
    BOOLEAN,
    STRING,
    SUBRANGE,
    ENUMERATED,
    ARRAY,
    RECORD
};

string simpleTypeToString(SimpleType type) {
    switch(type) {
        case REAL : return "real";
        case INTEGER : return "integer";
        case CHAR : return "char";
        case BOOLEAN : return "boolean";
        case STRING : return "string";
        case SUBRANGE : return "subrange";
        case ENUMERATED : return "enumerated";
        case ARRAY : return "array";
        case RECORD : return "record";
        default : return "unrecognized";
    }
}