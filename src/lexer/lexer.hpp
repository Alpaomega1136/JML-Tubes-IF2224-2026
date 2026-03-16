#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include "token.hpp"
#include <string>
#include <fstream>
#include <cctype>
using namespace std;

enum State {
    STATE_START,
    STATE_IDENT,      // membaca identifier/keyword
    STATE_INT,        // membaca angka integer
    STATE_REAL,       // membaca angka riil (setelah titik)
    STATE_STRING,     // membaca string/charcon (setelah tanda ')
    STATE_COLON,      // membaca ':' 
    STATE_LT,         // membaca '<' 
    STATE_GT,         // membaca '>' 
    STATE_EQL,        // membaca '=' 
    STATE_COMMENT1,   // membaca '{' → komentar
    STATE_COMMENT2,   // membaca '(*' → komentar
    // ...
};

// Fungsi utama yanh berfungsi untuk mengubah isi file.txt menjadi list of token
vector<Token> tokenize(const std::string& filename);

// Fungsi mengubah ListToken menjadi string dan memasukkkannya pada output.txt
string tokenTypeToString(ListToken list, const std::string& value);

// Fungsi untuk mengecek apakah suatu kata adalah keyword
ListToken  checkKeyword(const std::string& word);

#endif

