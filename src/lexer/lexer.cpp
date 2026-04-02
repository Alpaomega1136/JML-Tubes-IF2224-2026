#include "lexer.hpp"
#include <iostream>

bool isLetterEqual(char l, const char c) {
    return (l == c) || (l >= 65 && l <=90 && ((l + 32) == c));
}

bool caseInsensitiveCheck(char l, const char c) {
    return isLetterEqual(l,c) || isLetterEqual(c,l);
}

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isDelimiter(char c) {
    switch(c) {
        case ' ': 
        case '\n':
        case '+': 
        case '-': 
        case '*':
        case '/':
        case '=':
        case ',': 
        case ';':
        case ':':
        case '.':
        case '(':
        case ')':
        case '[':
        case ']':
        case '<':
        case '>':
        case '{':
        case '\'': return true;
        default: return false;
    }
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
    while(!file.eof()) {
        file.get(curr_char);
        if (file.eof()) {   
            curr_char = '\n';
        }
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
                    curr_state = EQL_STATE;
                    pushtoken(eql);
                    continue; 
                }
                curr_state = UNKNOWN2_STATE; //untuk kasus ini, menurutku di switch ini ditambah case UNKNOWN2_STATE dibawah case ini, terus breaknya dihapus. UNKNOWN2_STATE belum dibuat 
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
                break;
            case LPARENT_STATE: 
                if (curr_char == '*') {  
                    curr_state = COMMENT_STATE;
                    continue; 
                }
                pushtoken(lparent); 
                break;
            case RPARENT_STATE: 
                pushtoken(rparent);
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
            case START_QUOTE_STATE:
                switch(curr_char) {
                    case '\'':
                        curr_state = EMPTY_STRING_STATE;
                        break;
                    case '\n':
                        curr_value += '\'';
                        pushtoken(unknown);
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
                    case '\n':
                        curr_value = '\'' + curr_value;
                        pushtoken(unknown);
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
            case COMMENT_STATE :
                switch(curr_char) {
                    case '}':
                        curr_state = COMMENT_END_STATE;
                        break;
                    case '*':
                        curr_state = COMMENT_ASTERISK_STATE;
                        break;
                    default:
                        curr_value += curr_char;
                        break;
                }
                break;
            case COMMENT_ASTERISK_STATE:
                switch(curr_char) {
                    case ')':
                        curr_state = COMMENT_END_STATE;
                        break;
                    case '*':
                        curr_value += curr_char;
                        break;
                    case '}':
                        curr_value += '*';
                        curr_state = COMMENT_END_STATE;
                        break;
                    default:
                        curr_value += '*';
                        curr_value += curr_char;
                        curr_state = COMMENT_STATE;
                        break;
                }
                break;
            case COMMENT_END_STATE:
                pushtoken(comment);
                break;
            case INT_STATE:
                if (curr_char == '.') {
                    curr_state = INT_PERIOD_STATE;
                } else if (isDigit(curr_char)) {
                    curr_value += curr_char;
                } else if (isDelimiter(curr_char)) {
                    pushtoken(intcon);
                } else {
                    curr_state = UNKNOWN_STATE;
                    curr_value += curr_char;
                }
                break;
            case INT_PERIOD_STATE:
                if (isDigit(curr_char)) {
                    curr_state = REAL_STATE;
                    curr_value += '.';
                    curr_value += curr_char;
                } else {
                    pushtoken(intcon);
                    pushtoken(period);
                }
                break;
            case REAL_STATE:
                if (isDigit(curr_char)) {
                    curr_value += curr_char;
                } else if (isDelimiter(curr_char)) {
                    pushtoken(realcon);
                } else {
                    curr_state = UNKNOWN_STATE;
                    curr_value += curr_state;
                }
                break;
            case UNKNOWN_STATE:
                if (isDelimiter(curr_char)) {
                    pushtoken(unknown);
                } else {
                    curr_value += curr_char;
                }
                break;


            //Identifier
            case IDENT_STATE:
                if (isalnum(curr_char)) {
                    curr_value += curr_char;
                    continue; 
                } else {
                    pushtoken(ident);
                    break;
                }


            //Mulai untuk huruf A
            case A_STATE:
                if (tolower(curr_char) == 'n') {
                    curr_state = AN_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (tolower(curr_char) == 'r') {
                    curr_state = AR_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case AN_STATE: 
                if (tolower(curr_char) == 'd') {
                    curr_state = AND_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case AND_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(andsy); 
                    break; 
                }

            case AR_STATE:
                if (tolower(curr_char) == 'r') {
                    curr_state = ARR_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case ARR_STATE:
                if (tolower(curr_char) == 'a') {
                    curr_state = ARRA_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case ARRA_STATE: 
                if (tolower(curr_char) == 'y') {
                    curr_state = ARRAY_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char) ) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case ARRAY_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(arraysy); 
                    break; 
                }

            //Mulai hutuf b
            case B_STATE: 
                if (tolower(curr_char) == 'e') {
                    curr_state = BE_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); break; 
                }

            case BE_STATE: 
                if (tolower(curr_char) == 'g') {
                    curr_state = BEG_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case BEG_STATE: 
                if (tolower(curr_char) == 'i') {
                    curr_state = BEGI_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case BEGI_STATE: 
                if (tolower(curr_char) == 'n') {
                    curr_state = BEGIN_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case BEGIN_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(beginsy); break; 
                }


            //Mulai huruf c
            case C_STATE: 
                if (tolower(curr_char) == 'a') {
                    curr_state = CA_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (tolower(curr_char) == 'o') {
                    curr_state = CO_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            // Jalur case
            case CA_STATE:
                if (tolower(curr_char) == 's') {
                    curr_state = CAS_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case CAS_STATE: 
                if (tolower(curr_char) == 'e') {
                    curr_state = CASE_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case CASE_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(casesy); 
                    break; 
                }

            // Jalur const
            case CO_STATE: 
                if (tolower(curr_char) == 'n') {
                    curr_state = CON_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case CON_STATE: 
                if (tolower(curr_char) == 's') {
                    curr_state = CONS_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case CONS_STATE: 
                if (tolower(curr_char) == 't') {
                    curr_state = CONST_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(ident); 
                    break; 
                }

            case CONST_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE; 
                    curr_value += curr_char; 
                    continue;
                } else {
                    pushtoken(constsy); 
                    break; 
                }
            //Mulai huruf d
            case D_STATE: 
                if (tolower(curr_char) == 'i') {
                    curr_state = DI_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'o') {
                    curr_state = DO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            // Jalur div
            case DI_STATE: 
                if (tolower(curr_char) == 'v') {
                    curr_state = DIV_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case DIV_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(idiv);
                    break;
                }

            // Jalur do atau downto
            case DO_STATE: 
                if (tolower(curr_char) == 'w') {
                    curr_state = DOW_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(dosy);
                    break;
                }

            case DOW_STATE: 
                if (tolower(curr_char) == 'n') {
                    curr_state = DOWN_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case DOWN_STATE: 
                if (tolower(curr_char) == 't') {
                    curr_state = DOWNT_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case DOWNT_STATE: 
                if (tolower(curr_char) == 'o') {
                    curr_state = DOWNTO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case DOWNTO_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(downtosy);
                    break;
                }

            //Mulai huruf e
            case E_STATE:
                if (tolower(curr_char) == 'l') {
                    curr_state = EL_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'n') {
                    curr_state = EN_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            // Jalur else
            case EL_STATE: 
                if (tolower(curr_char) == 's') {
                    curr_state = ELS_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case ELS_STATE: 
                if (tolower(curr_char) == 'e') {
                    curr_state = ELSE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case ELSE_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(elsesy);
                    break;
                }

            // Jalur end
            case EN_STATE: 
                if (tolower(curr_char) == 'd') {
                    curr_state = END_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case END_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(endsy);
                    break;
                }

            //Mulai huruf f
            case F_STATE:
                if (tolower(curr_char) == 'o') {
                    curr_state = FO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'u') {
                    curr_state = FU_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            // Jalur for
            case FO_STATE: 
                if (tolower(curr_char) == 'r') {
                    curr_state = FOR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FOR_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(forsy);
                    break;
                }

            // Jalur function
            case FU_STATE: 
                if (tolower(curr_char) == 'n') {
                    curr_state = FUN_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FUN_STATE: 
                if (tolower(curr_char) == 'c') {
                    curr_state = FUNC_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FUNC_STATE: 
                if (tolower(curr_char) == 't') {
                    curr_state = FUNCT_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FUNCT_STATE: 
                if (tolower(curr_char) == 'i') {
                    curr_state = FUNCTI_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FUNCTI_STATE:
                if (tolower(curr_char) == 'o') {
                    curr_state = FUNCTIO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FUNCTIO_STATE:
                if (tolower(curr_char) == 'n') {
                    curr_state = FUNCTION_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case FUNCTION_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(functionsy);
                    break;
                }

            //Mulai huruf i
            //Jalur if
            case I_STATE:
                if (tolower(curr_char) == 'f') {
                    curr_state = IF_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case IF_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ifsy);
                    break;
                }

            //Mulai huruf m
            //Jalur mod
            case M_STATE:
                if (tolower(curr_char) == 'o') {
                    curr_state = MO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case MO_STATE:
                if (tolower(curr_char) == 'd') {
                    curr_state = MOD_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case MOD_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(imod);
                    break;
                }

            //Mulai huruf n
            //Jalur not
            case N_STATE: 
                if (tolower(curr_char) == 'o') {
                    curr_state = NO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case NO_STATE: 
                if (tolower(curr_char) == 't') {
                    curr_state = NOT_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case NOT_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(notsy);
                    break;
                }

            //Mulai huruf o
            //Jalur of dan or
            case O_STATE:
                if (tolower(curr_char) == 'f') {
                    curr_state = OF_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'r') {
                    curr_state = OR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case OF_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ofsy);
                    break;
                }

            case OR_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(orsy);
                    break;
                }

            //Mulai huruf p
            case P_STATE:
                if (tolower(curr_char) == 'r') {
                    curr_state = PR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PR_STATE:
                if (tolower(curr_char) == 'o') {
                    curr_state = PRO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PRO_STATE:
                if (tolower(curr_char) == 'c') {
                    curr_state = PROC_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'g') {
                    curr_state = PROG_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            //Jalur procedure
            case PROC_STATE: // "PROC"
                if (tolower(curr_char) == 'e') {
                    curr_state = PROCE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROCE_STATE: 
                if (tolower(curr_char) == 'd') {
                    curr_state = PROCED_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROCED_STATE:
                if (tolower(curr_char) == 'u') {
                    curr_state = PROCEDU_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROCEDU_STATE:
                if (tolower(curr_char) == 'r') {
                    curr_state = PROCEDUR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROCEDUR_STATE:
                if (tolower(curr_char) == 'e') {
                    curr_state = PROCEDURE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROCEDURE_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(proceduresy);
                    break;
                }

            //Jalur program
            case PROG_STATE: 
                if (tolower(curr_char) == 'r') {
                    curr_state = PROGR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROGR_STATE:
                if (tolower(curr_char) == 'a') {
                    curr_state = PROGRA_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROGRA_STATE:
                if (tolower(curr_char) == 'm') {
                    curr_state = PROGRAM_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case PROGRAM_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(programsy);
                    break;
                }

            //Mulai huruf r
            case R_STATE:
                if (tolower(curr_char) == 'e') {
                    curr_state = RE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case RE_STATE: 
                if (tolower(curr_char) == 'c') {
                    curr_state = REC_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'p') {
                    curr_state = REP_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            //Jalur record
            case REC_STATE: 
                if (tolower(curr_char) == 'o') {
                    curr_state = RECO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case RECO_STATE:
                if (tolower(curr_char) == 'r') {
                    curr_state = RECOR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case RECOR_STATE:
                if (tolower(curr_char) == 'd') {
                    curr_state = RECORD_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case RECORD_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(recordsy);
                    break;
                }

            // Jalur repeat
            case REP_STATE: 
                if (tolower(curr_char) == 'e') {
                    curr_state = REPE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case REPE_STATE:
                if (tolower(curr_char) == 'a') {
                    curr_state = REPEA_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case REPEA_STATE:
                if (tolower(curr_char) == 't') {
                    curr_state = REPEAT_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case REPEAT_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(repeatsy);
                    break;
                }

            //Mulai huruf t
            case T_STATE:
                if (tolower(curr_char) == 'h') {
                    curr_state = TH_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'o') {
                    curr_state = TO_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (tolower(curr_char) == 'y') {
                    curr_state = TY_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            // Jalur then
            case TH_STATE: 
                if (tolower(curr_char) == 'e') {
                    curr_state = THE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case THE_STATE:
                if (tolower(curr_char) == 'n') {
                    curr_state = THEN_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case THEN_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(thensy);
                    break;
                }

            // Jalur to
            case TO_STATE: 
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(tosy);
                    break;
                }

            // Jalur: type
            case TY_STATE: 
                if (tolower(curr_char) == 'p') {
                    curr_state = TYP_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case TYP_STATE:
                if (tolower(curr_char) == 'e') {
                    curr_state = TYPE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case TYPE_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(typesy);
                    break;
                }

            //Mulai huruf u
            //Jalur until
            case U_STATE:
                if (tolower(curr_char) == 'n') {
                    curr_state = UN_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case UN_STATE:
                if (tolower(curr_char) == 't') {
                    curr_state = UNT_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case UNT_STATE:
                if (tolower(curr_char) == 'i') {
                    curr_state = UNTI_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case UNTI_STATE:
                if (tolower(curr_char) == 'l') {
                    curr_state = UNTIL_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case UNTIL_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(untilsy);
                    break;
                }
            
            //Mulai huruf v
            //Jalur var
            case V_STATE:
                if (tolower(curr_char) == 'a') {
                    curr_state = VA_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case VA_STATE:
                if (tolower(curr_char) == 'r') {
                    curr_state = VAR_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case VAR_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(varsy);
                    break;
                }

            //Mulai huruf w
            //Jalur while
            case W_STATE:
                if (tolower(curr_char) == 'h') {
                    curr_state = WH_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case WH_STATE:
                if (tolower(curr_char) == 'i') {
                    curr_state = WHI_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case WHI_STATE:
                if (tolower(curr_char) == 'l') {
                    curr_state = WHIL_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case WHIL_STATE: 
                if (tolower(curr_char) == 'e') {
                    curr_state = WHILE_STATE;
                    curr_value += curr_char;
                    continue;
                } else if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(ident);
                    break;
                }

            case WHILE_STATE:
                if (isalnum(curr_char)) {
                    curr_state = IDENT_STATE;
                    curr_value += curr_char;
                    continue;
                } else {
                    pushtoken(whilesy);
                    break;
                }
            case UNKNOWN2_STATE:
                if (isDelimiter(curr_char) || isalnum(curr_char)) {
                    pushtoken(unknown);
                }
                else {
                    curr_value += curr_char;
                }
                break;
            default:
                break;
        }

        if(curr_state == START_STATE) {
            switch (curr_char) {
                case '+': curr_state = PLUS_STATE;      curr_value += curr_char;    ;break;
                case '-': curr_state = MINUS_STATE;     curr_value += curr_char;    ;break;
                case '*': curr_state = TIMES_STATE;     curr_value += curr_char;    ;break;
                case '/': curr_state = RDIV_STATE;      curr_value += curr_char;    ;break;
                case '=': curr_state = EQL_STATE;       curr_value += curr_char;    ;break;
                case ',': curr_state = COMMA_STATE;     curr_value += curr_char;    ;break;
                case ';': curr_state = SEMICOLON_STATE; curr_value += curr_char;    ;break;
                case ':': curr_state = COLON_STATE;     curr_value += curr_char;    ;break;
                case '.': curr_state = PERIOD_STATE;    curr_value += curr_char;    ;break;
                case '(': curr_state = LPARENT_STATE;   ;break;
                case ')': curr_state = RPARENT_STATE;   curr_value += curr_char;    ;break;
                case '[': curr_state = LBRACK_STATE;    curr_value += curr_char;    ;break;
                case ']': curr_state = RBRACK_STATE;    curr_value += curr_char;    ;break;
                case '<': curr_state = LSS_STATE;       curr_value += curr_char;    ;break;
                case '>': curr_state = GTR_STATE;       curr_value += curr_char;    ;break;
                case '{': curr_state = COMMENT_STATE; break;
                case '\'': curr_state = START_QUOTE_STATE; break;
                default:
                    if (isDigit(curr_char)) {
                        curr_state = INT_STATE;
                        curr_value += curr_char;
                        break;
                    }
                    if (isalpha(curr_char)) {
                        curr_value += curr_char;
                        switch (tolower(curr_char)) {
                            case 'a': curr_state = A_STATE; break;
                            case 'b': curr_state = B_STATE; break;
                            case 'c': curr_state = C_STATE; break;
                            case 'd': curr_state = D_STATE; break;
                            case 'e': curr_state = E_STATE; break;
                            case 'f': curr_state = F_STATE; break;
                            case 'i': curr_state = I_STATE; break;
                            case 'm': curr_state = M_STATE; break;
                            case 'n': curr_state = N_STATE; break;
                            case 'o': curr_state = O_STATE; break;
                            case 'p': curr_state = P_STATE; break;
                            case 'r': curr_state = R_STATE; break;
                            case 't': curr_state = T_STATE; break;
                            case 'u': curr_state = U_STATE; break;
                            case 'v': curr_state = V_STATE; break;
                            case 'w': curr_state = W_STATE; break;
                            default:  curr_state = IDENT_STATE; break;
                        }
                        break;
                    }
                    if (!isDelimiter(curr_char)) {
                        curr_value += curr_char;
                        curr_state = UNKNOWN2_STATE;
                    }
                    break;
            }
        }
    }
    return tokens;
}