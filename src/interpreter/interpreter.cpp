#include "interpreter.hpp"

RuntimeError::RuntimeError(const std::string& message)
    : std::runtime_error(message) {}

Interpreter::Interpreter(RuntimeConfig config)
    : config(config) {}

RuntimeValue::RuntimeValue(std::int32_t value)
    : integer(value) {}

void Interpreter::reset() {
    stack.clear();
    memorySize = 0;
}

void Interpreter::pushValue(RuntimeValue value) {
    stack.push_back(value);
}

RuntimeValue Interpreter::popValue() {
    RuntimeValue value = stack.back();
    stack.pop_back();
    return value;
}

void Interpreter::execute(const std::vector<Instruction>& instructions,
                          std::ostream& out) {
    reset();
    (void)instructions;
    (void)out;
}
