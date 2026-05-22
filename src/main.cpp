#include <iostream>
#include <fstream>
#include <filesystem>
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include "semantic_analysis/ast.hpp"
#include "semantic_analysis/ast_printer.hpp"
#include "semantic_analysis/semantic_analyzer.hpp"

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
        ASTNode* ast = buildAST(parseTree);
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast);

        std::filesystem::path path(outputFile);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream output(outputFile);
        if (!output.is_open()) {
            throw std::runtime_error("failed to open output file: " + outputFile);
        }

        ASTPrinter printer;
        auto writeSemanticOutput = [&](std::ostream& out) {
            if (analyzer.hasErrors()) {
                out << "=== Semantic Errors ===" << endl;
                analyzer.printErrors(out);
                out << endl;
            }

            out << "=== Decorated AST Tree ===" << endl;
            printer.print(ast, out);

            out << endl;
            out << "=== Symbol Tables ===" << endl;
            analyzer.printSymbolTables(out);
        };

        writeSemanticOutput(cout);
        writeSemanticOutput(output);
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
