#include "lexer.hpp"

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
        switch(curr_state) {
            case START_QUOTE_STATE:
                switch(curr_char) {
                    case '\'':
                        curr_state = EMPTY_STRING_STATE;
                    default :
                        curr_state = CHAR_STATE;
                        curr_value += curr_char;
                }
            case CHAR_STATE:
                switch(curr_char) {
                    case '\'':
                        curr_state = CHAR_END_STATE;
                    default :
                        curr_state = STRING_ONGOING_STATE;
                        curr_value += curr_char;
                }
            case CHAR_END_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_ONGOING_STATE;
                        curr_value += curr_char;
                    default:
                        pushtoken(charcon);
                }
            case STRING_ONGOING_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_END_STATE;
                    default:
                        curr_value += curr_char;
                }
            case EMPTY_STRING_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = CHAR_STATE;
                        curr_value += curr_char;
                    default :
                        pushtoken(stringcon);
                }
            case STRING_END_STATE :
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_ONGOING_STATE;
                        curr_value += curr_char;
                    default:
                        pushtoken(stringcon);
                }
        }

        if (curr_state == START_STATE) {
            switch(curr_char) {
                case '\'':
                    curr_state = START_QUOTE_STATE;
            }   
        }
    }
    return tokens;
}

string tokenTypeToString(const Token& t) {
    switch(t.type) {
        case stringcon:
            return "stringcon ('" + t.value +"')";
        case charcon:
            return "charcon ('" + t.value +"')";
    }
};

ListToken checkKeyword(const std::string& word);