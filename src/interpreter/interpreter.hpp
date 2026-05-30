#pragma once

#include "../codegen/codegen.hpp"
#include <iosfwd>
#include <vector>

class Interpreter {
public:
    void execute(const std::vector<Instruction>& instructions, std::ostream& out);
};
