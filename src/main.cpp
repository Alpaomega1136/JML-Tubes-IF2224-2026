#include <iostream>
#include <fstream>
#include <filesystem>
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"

using namespace std;

int main(int argc, char* argv[]){
    if(argc < 3) {
        cout << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];

    try {
        vector<Token> tokens = tokenize(inputFile);
        Parser parser(tokens);
        TreeParser* parseTree = parser.parse();
        parser.printParseTree(parseTree, outputFile);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::filesystem::path path(outputFile);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        std::ofstream output(outputFile);
        if (output.is_open()) {
            output << e.what() << std::endl;
        }
        return 1;
    }
}
