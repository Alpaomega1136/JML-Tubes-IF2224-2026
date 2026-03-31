#include "lexer.hpp"
#include <iostream>

bool isLetter(char l, const char c) {
    return (l == c) || (l >= 65 && l <=90 && ((l + 32) == c));
}

bool caseInsensitiveCheck(char l, const char c) {
    return isLetter(l,c) || isLetter(c,l);
}

vector<Token> tokenize(const std::string& filename) {
    ifstream file(filename);
    char curr_char;
    State curr_state = START_STATE;
    std::string curr_value = "";
    vector<Token> tokens;
    auto pushtoken = [&](ListToken type){
        tokens.push_back(Token(type, curr_value));
        curr_state = START_STATE;
        curr_value ="";
    };
    while(file.get(curr_char)) {
        switch (curr_state){
            case PLUS_STATE: 
                pushtoken(ListToken::plus);      
                break;
            case MINUS_STATE: 
                pushtoken(ListToken::minus);     
                break;
            case TIMES_STATE: 
                pushtoken(times);     
                break;
            case RDIV_STATE: 
                pushtoken(rdiv);      
                break;
            case SEMI_EQL_STATE: 
                if (curr_char == '=') {
                    curr_value += curr_char;
                    pushtoken(eql); 
                    continue; 
                }
                pushtoken(eql);
                break;
            case COMMA_STATE: 
                pushtoken(comma);     
                break;
            case SEMICOLON_STATE: 
                pushtoken(semicolon); 
                break;
            case COLON_STATE: 
                if (curr_char == '=') {
                    curr_value += curr_char;
                    pushtoken(becomes); 
                    continue;
                }
                pushtoken(colon); 
                break;
            case PERIOD_STATE: 
                pushtoken(period);
            case LPARENT_STATE: 
                if (curr_char == '*') {  
                    curr_state = COMMENT2_STATE;
                    continue; 
                }
                pushtoken(lparent); 
                break;
            case RPARENT_STATE: 
                pushtoken(lbrack);
                break;
            case LBRACK_STATE: 
                pushtoken(lbrack);    
                break;
            case RBRACK_STATE: 
                pushtoken(rbrack);    
                break;
            case LSS_STATE: 
                if (curr_char == '=') { 
                    curr_value += curr_char;
                    pushtoken(leq); 
                    continue; 
                }
                if (curr_char == '>') { 
                    curr_value +=curr_char;
                    pushtoken(neq); 
                    continue; 
                }
                pushtoken(lss); 
                break;
            case GTR_STATE: 
                if (curr_char == '=') { 
                    curr_value += curr_char;
                    pushtoken(geq); 
                    continue; 
                }
                pushtoken(gtr); 
                break;
        }

        switch(curr_state){
            case START_STATE:
                switch (curr_char) {
                    case '+': curr_state = PLUS_STATE;      curr_value += curr_char;    ;break;
                    case '-': curr_state = MINUS_STATE;     curr_value += curr_char;    ;break;
                    case '*': curr_state = TIMES_STATE;     curr_value += curr_char;    ;break;
                    case '/': curr_state = RDIV_STATE;      curr_value += curr_char;    ;break;
                    case '=': curr_state = SEMI_EQL_STATE;  curr_value += curr_char;    ;break;
                    case ',': curr_state = COMMA_STATE;     curr_value += curr_char;    ;break;
                    case ';': curr_state = SEMICOLON_STATE; curr_value += curr_char;    ;break;
                    case ':': curr_state = COLON_STATE;     curr_value += curr_char;    ;break;
                    case '.': curr_state = PERIOD_STATE;    curr_value += curr_char;    ;break;
                    case '(': curr_state = LPARENT_STATE;   curr_value += curr_char;    ;break;
                    case ')': curr_state = RPARENT_STATE;   curr_value += curr_char;    ;break;
                    case '[': curr_state = LBRACK_STATE;    curr_value += curr_char;    ;break;
                    case ']': curr_state = RBRACK_STATE;    curr_value += curr_char;    ;break;
                    case '<': curr_state = LSS_STATE;       curr_value += curr_char;    ;break;
                    case '>': curr_state = GTR_STATE;       curr_value += curr_char;    ;break;
                }
            case START_QUOTE_STATE:
                switch(curr_char) {
                    case '\'':
                        curr_state = EMPTY_STRING_STATE;
                        break;
                    default :
                        curr_state = CHAR_STATE;
                        curr_value += curr_char;
                        break;
                }
                break;
            case CHAR_STATE:
                switch(curr_char) {
                    case '\'':
                        curr_state = CHAR_END_STATE;
                        break;
                    default :
                        curr_state = STRING_ONGOING_STATE;
                        curr_value += curr_char;
                        break;
                }
                break;
            case CHAR_END_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_ONGOING_STATE;
                        curr_value += curr_char;
                        break;
                    default:
                        pushtoken(charcon);
                        break;
                }
                break;
            case STRING_ONGOING_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_END_STATE;
                        break;
                    default:
                        curr_value += curr_char;
                        break;
                }
                break;
            case EMPTY_STRING_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = CHAR_STATE;
                        curr_value += curr_char;
                        break;
                    default :
                        pushtoken(stringcon);
                        break;
                }
                break;
            case STRING_END_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_ONGOING_STATE;
                        curr_value += curr_char;
                        break;
                    default:
                        pushtoken(stringcon);
                        break;
                }
                break;
            default:
                break;
        }

        if (curr_state == START_STATE) {
            switch(curr_char) {
                case '\'':
                    curr_state = START_QUOTE_STATE;
                default:
                    break;
            }
        }
    }
    return tokens;
}

string tokenTypeToString(const Token& t) {
    switch(t.type) {
        case stringcon:
            return "stringcon (" + t.value +")";
        case charcon:
            return "charcon (" + t.value +")";
        default:
            return "how bro";
    }
};

ListToken checkKeyword(const std::string& word);