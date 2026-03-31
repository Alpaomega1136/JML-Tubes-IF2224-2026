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
    auto pushtoken = [&](ListToken type, string val){
        tokens.push_back(Token(type, val));
        curr_state = START_STATE;
        curr_value ="";
    };
    while(file.get(curr_char)) {
        switch (curr_state){
            case PLUS_STATE: 
                pushtoken(ListToken::plus, "+");      
                break;
            case MINUS_STATE: 
                pushtoken(ListToken::minus, "-");     
                break;
            case TIMES_STATE: 
                pushtoken(times, "*");     
                break;
            case RDIV_STATE: 
                pushtoken(rdiv, "/");      
                break;
            case EQL_STATE: 
                if (curr_char == '=') {
                    pushtoken(eql, "=="); 
                    continue; 
                }
                pushtoken(eql, "=");
                break;
            case COMMA_STATE: 
                pushtoken(comma, ",");     
                break;
            case SEMICOLON_STATE: 
                pushtoken(semicolon, ";"); 
                break;
            case COLON_STATE: 
                if (curr_char == '=') {
                    pushtoken(becomes, ":="); 
                    continue;
                }
                pushtoken(colon, ":"); 
                break;
            case PERIOD_STATE: 
                pushtoken(period, ".");
            case LPARENT_STATE: 
                if (curr_char == '*') {  
                    curr_state = COMMENT2_STATE;
                    continue; 
                }
                pushtoken(lparent, "("); 
                break;
            case RPARENT_STATE: 
                pushtoken(lbrack, "[");
                break;
            case LBRACK_STATE: 
                pushtoken(lbrack, "[");    
                break;
            case RBRACK_STATE: 
                pushtoken(rbrack, "]");    
                break;
            case LSS_STATE: 
                if (curr_char == '=') { 
                    pushtoken(leq, "<="); 
                    continue; 
                }
                if (curr_char == '>') { 
                    pushtoken(neq, "<>"); 
                    continue; 
                }
                pushtoken(lss, "<"); 
                break;
            case GTR_STATE: 
                if (curr_char == '=') { 
                    pushtoken(geq, ">="); 
                    continue; 
                }
                pushtoken(gtr, ">"); 
                break;
        }

        switch(curr_state){
            case START_STATE:
                switch (curr_char) {
                    case '+': curr_state = PLUS_STATE;      break;
                    case '-': curr_state = MINUS_STATE;     break;
                    case '*': curr_state = TIMES_STATE;     break;
                    case '/': curr_state = RDIV_STATE;      break;
                    case '=': curr_state = EQL_STATE;       break;
                    case ',': curr_state = COMMA_STATE;     break;
                    case ';': curr_state = SEMICOLON_STATE; break;
                    case ':': curr_state = COLON_STATE;     break;
                    case '.': curr_state = PERIOD_STATE;    break;
                    case '(': curr_state = LPARENT_STATE;   break;
                    case ')': curr_state = RPARENT_STATE;   break;
                    case '[': curr_state = LBRACK_STATE;    break;
                    case ']': curr_state = RBRACK_STATE;    break;
                    case '<': curr_state = LSS_STATE;       break;
                    case '>': curr_state = GTR_STATE;       break;
                }
            case START_QUOTE_STATE:
                switch(curr_char) {
                    case '\'':
                        curr_state = STRING_END_STATE;
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
        }
    }
}

string tokenTypeToString(ListToken list, const std::string& value);

ListToken checkKeyword(const std::string& word);