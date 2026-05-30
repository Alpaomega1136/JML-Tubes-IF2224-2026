#pragma once

#include "../codegen/codegen.hpp"
#include <cstddef>
#include <iosfwd>
#include <stdexcept>
#include <vector>

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& message);
};

struct RuntimeConfig {
    std::size_t maxStackSize = 4096;
    std::size_t maxSteps = 1000000;
};

class Interpreter {
public:
    explicit Interpreter(RuntimeConfig config = RuntimeConfig());

    void execute(const std::vector<Instruction>& instructions, std::ostream& out);

private:
    RuntimeConfig config;
};
