CXX      = g++
CXXFLAGS = -Wall -std=c++17 -I src
BIN_DIR  = bin
TARGET   = $(BIN_DIR)/arion
SRCS     = src/main.cpp \
           src/lexer/lexer.cpp \
           src/lexer/token.cpp \
           src/parser/parser.cpp \
           src/semantic_analysis/ast.cpp \
           src/semantic_analysis/ast_printer.cpp \
           src/semantic_analysis/symbol_table.cpp \
           src/semantic_analysis/semantic_analyzer.cpp
OBJS     = $(patsubst src/%.cpp,$(BIN_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(BIN_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)
	rm -f lexer
