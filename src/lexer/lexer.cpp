#include "lexer.hpp"

vector<Token> tokenize(const std::string& filename);

string tokenTypeToString(ListToken list, const std::string& value);

ListToken  checkKeyword(const std::string& word);