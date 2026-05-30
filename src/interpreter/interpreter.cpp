#include "interpreter.hpp"

RuntimeError::RuntimeError(const std::string& message)
    : std::runtime_error(message) {}

Interpreter::Interpreter(RuntimeConfig config)
    : config(config) {}

RuntimeValue::RuntimeValue(std::int32_t value)
    : integer(value) {}

void Interpreter::execute(const std::vector<Instruction>& instructions,
                          std::ostream& out) {
    (void)instructions;
    (void)out;
}
