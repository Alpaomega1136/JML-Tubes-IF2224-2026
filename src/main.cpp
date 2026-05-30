#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "codegen/codegen.hpp"
#include "interpreter/interpreter.hpp"
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

        string intermediateCode;
        string programOutput;
        string runtimeError;

        if (!analyzer.hasErrors()) {
            CodeGenerator generator;
            generator.generate(ast);

            std::ostringstream icStream;
            generator.print(icStream);
            intermediateCode = icStream.str();

            std::ostringstream runtimeStream;
            try {
                Interpreter interpreter;
                interpreter.execute(generator.getInstructions(), runtimeStream);
                programOutput = runtimeStream.str();
            } catch (const RuntimeError& e) {
                programOutput = runtimeStream.str();
                runtimeError = e.what();
            }
        }

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

            if (!analyzer.hasErrors()) {
                out << endl;
                out << "=== Intermediate Code ===" << endl;
                out << intermediateCode;

                out << endl;
                out << "=== Program Output ===" << endl;
                out << programOutput;
                if (!programOutput.empty() && programOutput.back() != '\n') {
                    out << endl;
                }

                if (!runtimeError.empty()) {
                    out << endl;
                    out << "=== Runtime Error ===" << endl;
                    out << runtimeError << endl;
                }
            }
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
