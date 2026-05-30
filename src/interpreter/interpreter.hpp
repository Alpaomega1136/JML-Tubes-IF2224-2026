#pragma once

#include "../codegen/codegen.hpp"
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& message);
};

struct RuntimeConfig {
    std::size_t maxStackSize = 4096;
    std::size_t maxSteps = 1000000;
};

struct RuntimeValue {
    std::int32_t integer = 0;

    RuntimeValue() = default;
    explicit RuntimeValue(std::int32_t value);
};

class Interpreter {
public:
    explicit Interpreter(RuntimeConfig config = RuntimeConfig());

    void execute(const std::vector<Instruction>& instructions, std::ostream& out);

private:
    RuntimeConfig config;
    std::vector<RuntimeValue> stack;
    std::size_t memorySize = 0;

    void reset();
    void pushValue(RuntimeValue value);
    RuntimeValue popValue();
};
