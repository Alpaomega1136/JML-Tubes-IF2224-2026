#include <iostream>
#include <fstream>
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"

using namespace std;

int main(int argc, char* argv[]){
    if(argc < 3) {
        cout << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];

    vector<Token> tokens = tokenize(inputFile);
    ofstream out(outputFile);
    for (const Token& t : tokens) {
        out << t.tokenTypeToString() << "\n";
    }
    std::cout << "Tokenisasi selesai!" << std::endl;
    return 0;
}

