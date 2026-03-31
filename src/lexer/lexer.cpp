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
    while(file.get(curr_char)) {
        switch(curr_state){
            case START_STATE:
                switch(curr_char) {
                    
                }
        }
    }
}

string tokenTypeToString(ListToken list, const std::string& value);

ListToken checkKeyword(const std::string& word);