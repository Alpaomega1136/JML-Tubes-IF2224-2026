#include "interpreter.hpp"

#include <limits>
#include <ostream>

RuntimeError::RuntimeError(const std::string& message)
    : std::runtime_error(message) {}

Interpreter::Interpreter(RuntimeConfig config)
    : config(config) {}

RuntimeValue::RuntimeValue(std::int32_t value)
    : integer(value) {}

void Interpreter::reset() {
    stack.clear();
    memorySize = 0;
    pc = 0;
    steps = 0;
    halted = false;
}

const Instruction& Interpreter::fetch(
    const std::vector<Instruction>& instructions
) const {
    if (pc >= instructions.size()) {
        throw RuntimeError("Runtime Error: instruction pointer out of bounds");
    }
    return instructions[pc];
}

void Interpreter::executeInstruction(
    const Instruction& instruction,
    const std::vector<Instruction>& instructions,
    std::ostream& out
) {
    (void)instructions;
    (void)out;

    switch (instruction.opcode) {
        case Opcode::INT:
            initializeMemory(instruction.operand);
            break;
        case Opcode::LIT:
            pushValue(RuntimeValue(instruction.operand));
            break;
        case Opcode::LOD:
            pushValue(readMemory(instruction.operand));
            break;
        case Opcode::STO:
            writeMemory(instruction.operand, popValue());
            break;
        case Opcode::OPR:
            executeOpr(instruction.operand, out);
            break;
        case Opcode::RET:
            halted = true;
            break;
        case Opcode::JMP:
        case Opcode::JPC:
        case Opcode::CAL:
            validateInstructionTarget(instruction, instructions);
            throw RuntimeError(
                "Runtime Error: instruction " +
                CodeGenerator::opcodeToString(instruction.opcode) +
                " is not supported in Orang 2 interpreter core"
            );
        default:
            throw RuntimeError(
                "Runtime Error: invalid opcode at line " +
                std::to_string(instruction.line)
            );
    }
}

void Interpreter::executeOpr(int operation, std::ostream& out) {
    (void)out;

    switch (static_cast<OprCode>(operation)) {
        case OprCode::NEG: {
            RuntimeValue value = popValue();
            pushValue(RuntimeValue(checkedInt32(
                -static_cast<std::int64_t>(value.integer),
                "NEG"
            )));
            break;
        }
        case OprCode::ADD:
        case OprCode::SUB:
        case OprCode::MUL:
        case OprCode::DIV:
        case OprCode::MOD: {
            RuntimeValue right = popValue();
            RuntimeValue left = popValue();
            std::int64_t result = 0;
            std::string operationName;

            if (static_cast<OprCode>(operation) == OprCode::ADD) {
                result = static_cast<std::int64_t>(left.integer) + right.integer;
                operationName = "ADD";
            } else if (static_cast<OprCode>(operation) == OprCode::SUB) {
                result = static_cast<std::int64_t>(left.integer) - right.integer;
                operationName = "SUB";
            } else if (static_cast<OprCode>(operation) == OprCode::MUL) {
                result = static_cast<std::int64_t>(left.integer) * right.integer;
                operationName = "MUL";
            } else if (static_cast<OprCode>(operation) == OprCode::DIV) {
                if (right.integer == 0) {
                    throw RuntimeError("Runtime Error: Division by zero");
                }
                result = static_cast<std::int64_t>(left.integer) / right.integer;
                operationName = "DIV";
            } else {
                if (right.integer == 0) {
                    throw RuntimeError("Runtime Error: Modulo by zero");
                }
                result = static_cast<std::int64_t>(left.integer) % right.integer;
                operationName = "MOD";
            }

            pushValue(RuntimeValue(checkedInt32(result, operationName)));
            break;
        }
        case OprCode::EQL:
        case OprCode::NEQ:
        case OprCode::LSS:
        case OprCode::GEQ:
        case OprCode::GTR:
        case OprCode::LEQ: {
            RuntimeValue right = popValue();
            RuntimeValue left = popValue();
            bool result = false;

            if (static_cast<OprCode>(operation) == OprCode::EQL) {
                result = left.integer == right.integer;
            } else if (static_cast<OprCode>(operation) == OprCode::NEQ) {
                result = left.integer != right.integer;
            } else if (static_cast<OprCode>(operation) == OprCode::LSS) {
                result = left.integer < right.integer;
            } else if (static_cast<OprCode>(operation) == OprCode::GEQ) {
                result = left.integer >= right.integer;
            } else if (static_cast<OprCode>(operation) == OprCode::GTR) {
                result = left.integer > right.integer;
            } else {
                result = left.integer <= right.integer;
            }

            pushValue(RuntimeValue(result ? 1 : 0));
            break;
        }
        case OprCode::WRT: {
            RuntimeValue value = popValue();
            out << value.integer;
            break;
        }
        case OprCode::WRTLN: {
            RuntimeValue value = popValue();
            out << value.integer << '\n';
            break;
        }
        default:
            throw RuntimeError(
                "Runtime Error: invalid OPR code " +
                std::to_string(operation)
            );
    }
}

void Interpreter::validateInstructionTarget(
    const Instruction& instruction,
    const std::vector<Instruction>& instructions
) const {
    if (instruction.operand < 0 ||
        static_cast<std::size_t>(instruction.operand) >= instructions.size()) {
        throw RuntimeError(
            "Runtime Error: Label not found for " +
            CodeGenerator::opcodeToString(instruction.opcode) +
            " target " + std::to_string(instruction.operand)
        );
    }
}

std::int32_t Interpreter::checkedInt32(
    std::int64_t result,
    const std::string& operation
) const {
    if (result > std::numeric_limits<std::int32_t>::max()) {
        throw RuntimeError(
            "Runtime Error: Overflow during " + operation
        );
    }
    if (result < std::numeric_limits<std::int32_t>::min()) {
        throw RuntimeError(
            "Runtime Error: Underflow during " + operation
        );
    }
    return static_cast<std::int32_t>(result);
}

void Interpreter::initializeMemory(int size) {
    if (size < 0) {
        throw RuntimeError("Runtime Error: negative memory size");
    }

    if (static_cast<std::size_t>(size) > config.maxStackSize) {
        throw RuntimeError(
            "Runtime Error: Stack Overflow (initial memory exceeds limit " +
            std::to_string(config.maxStackSize) + ")"
        );
    }

    stack.assign(static_cast<std::size_t>(size), RuntimeValue());
    memorySize = static_cast<std::size_t>(size);
}

void Interpreter::validateAddress(int address) const {
    if (address < 0 || static_cast<std::size_t>(address) >= memorySize) {
        throw RuntimeError(
            "Runtime Error: Memory Access Out of Bounds at address " +
            std::to_string(address)
        );
    }
}

RuntimeValue Interpreter::readMemory(int address) const {
    validateAddress(address);
    return stack[static_cast<std::size_t>(address)];
}

void Interpreter::writeMemory(int address, RuntimeValue value) {
    validateAddress(address);
    stack[static_cast<std::size_t>(address)] = value;
}

void Interpreter::pushValue(RuntimeValue value) {
    if (stack.size() >= config.maxStackSize) {
        throw RuntimeError(
            "Runtime Error: Stack Overflow (limit " +
            std::to_string(config.maxStackSize) + ")"
        );
    }
    stack.push_back(value);
}

RuntimeValue Interpreter::popValue() {
    if (stack.size() <= memorySize) {
        throw RuntimeError("Runtime Error: Stack Underflow");
    }
    RuntimeValue value = stack.back();
    stack.pop_back();
    return value;
}

void Interpreter::execute(const std::vector<Instruction>& instructions,
                          std::ostream& out) {
    reset();

    while (!halted && pc < instructions.size()) {
        if (steps >= config.maxSteps) {
            throw RuntimeError(
                "Runtime Error: maximum execution step limit exceeded"
            );
        }

        const Instruction& instruction = fetch(instructions);
        pc++;
        steps++;
        executeInstruction(instruction, instructions, out);
    }
}
