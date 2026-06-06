#include "interpreter.hpp"

#include <limits>
#include <ostream>
#include <cstdint>

RuntimeError::RuntimeError(const std::string& message)
    : std::runtime_error(message) {}

Interpreter::Interpreter(RuntimeConfig config)
    : config(config) {}

RuntimeValue::RuntimeValue(std::int32_t value)
    : integer(value){}

RuntimeValue::RuntimeValue(pair<RuntimeType, std::int32_t> pair)
    : type(pair.first), integer(pair.second)  {}

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
    int ptrAddr;
    switch (instruction.opcode) {
        case Opcode::INT:
            initializeMemory(instruction.operand);
            break;
        case Opcode::LIT:
            pushValue(RuntimeValue(unwrapRuntimeType(instruction.operand)));
            break;
        case Opcode::LOD:
            pushValue(readMemory(instruction.operand));
            break;
        case Opcode::STO:
            writeMemory(instruction.operand, popValue());
            break;
        case Opcode::PLO:
            ptrAddr = popValue().integer;
            pushValue(readMemory(ptrAddr + instruction.operand));
            break;
        case Opcode::PST:
            ptrAddr = popValue().integer;
            writeMemory(ptrAddr, popValue());
            break;
        case Opcode::OPR:
            executeOpr(instruction.operand, out);
            break;
        case Opcode::RET:
            halted = true;
            break;
        case Opcode::JMP:
            validateInstructionTarget(instruction, instructions);
            pc = static_cast<std::size_t>(instruction.operand);
            break;
        case Opcode::JPC: {
            validateInstructionTarget(instruction, instructions);
            RuntimeValue condition = popValue();
            if (condition.integer == 0) {
                pc = static_cast<std::size_t>(instruction.operand);
            }
            break;
        }
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
            pushValue(RuntimeValue({value.type, checkedInt32(
                -static_cast<std::int64_t>(value.integer),
                "NEG"
            )}));
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
            RuntimeType resultType = resolveRuntimeType(left, right);
            if (resultType == RuntimeType::ERROR) {
                throw RuntimeError("Runtime Error: Unable to do operation with Runtime Type " +
                                    runtimeTypeToString(left.type) + " and " + runtimeTypeToString(right.type)
                                    );
            }
            std::string operationName;

            if (static_cast<OprCode>(operation) == OprCode::ADD) {
                if (resultType == RuntimeType::REAL) {
                    result = handleFloatingPointOperation(left, right, OprCode::ADD);
                } else {
                    result = static_cast<std::int64_t>(left.integer) + right.integer;
                }
                operationName = "ADD";
            } else if (static_cast<OprCode>(operation) == OprCode::SUB) {
                if (resultType == RuntimeType::REAL) {
                    result = handleFloatingPointOperation(left, right, OprCode::SUB);
                } else {
                    result = static_cast<std::int64_t>(left.integer) - right.integer;   
                }
                operationName = "SUB";
            } else if (static_cast<OprCode>(operation) == OprCode::MUL) {
                if (resultType == RuntimeType::REAL) {
                    result = handleFloatingPointOperation(left, right, OprCode::MUL);
                } else {
                    result = static_cast<std::int64_t>(left.integer) * right.integer;
                }
                operationName = "MUL";
            } else if (static_cast<OprCode>(operation) == OprCode::DIV) {
                if (right.integer == 0) {
                    throw RuntimeError("Runtime Error: Division by zero");
                }
                if (resultType == RuntimeType::REAL) {
                    result = handleFloatingPointOperation(left, right, OprCode::DIV);
                } else {
                    result = static_cast<std::int64_t>(left.integer) / right.integer;
                }
                operationName = "DIV";
            } else {
                if (right.integer == 0) {
                    throw RuntimeError("Runtime Error: Modulo by zero");
                }
                if (resultType == RuntimeType::REAL) {
                    throw RuntimeError(
                        "Runtime Error: Unsupported operation for Floating Point type"
                    );
                } else {
                    result = static_cast<std::int64_t>(left.integer) % right.integer;
                }
                operationName = "MOD";
            }

            pushValue(RuntimeValue({resultType, checkedInt32(result, operationName)}));
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
            RuntimeType resultType = resolveRuntimeType(left, right);
            if (resultType == RuntimeType::ERROR) {
                throw RuntimeError("Runtime Error: Unable to do operation with Runtime Type " +
                                    runtimeTypeToString(left.type) + " and " + runtimeTypeToString(right.type)
                                    );
            }

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

            pushValue(RuntimeValue({RuntimeType::BOOLEAN, result ? 1 : 0}));
            break;
        }
        case OprCode::WRT: {
            RuntimeValue value = popValue();
            out << resolveWriteValue(value);
            break;
        }
        case OprCode::WRTLN: {
            RuntimeValue value = popValue();
            out << resolveWriteValue(value) << '\n';
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

std::string Interpreter::resolveWriteValue(RuntimeValue val) {
    cout<<runtimeTypeToString(val.type)<<endl;
    switch(val.type) {
        case RuntimeType::INT :
            return to_string(val.integer);
        case RuntimeType::REAL : {
            uint32_t bit_pattern;
            float f;
            bit_pattern = static_cast<uint32_t>(val.integer);
            memcpy(&f, &bit_pattern, sizeof(float));
            return to_string(f);
        }
        case RuntimeType::BOOLEAN :
            return val.integer == 1 ? "true" : "false";
        case RuntimeType::CHAR :
            return string(1, (char)val.integer);
        case RuntimeType::STRING : {
            auto it = CodeGenerator::hashedStrings.find(val.integer);
            if (it == CodeGenerator::hashedStrings.end()) {
                return "";
            } else {
                return it->second;
            }
        }
        default :
            return "";
    }
    
}

RuntimeType Interpreter::resolveRuntimeType(RuntimeValue val1, RuntimeValue val2) {
    if (val1.type == val2.type) {
        return val1.type;
    } else {
        return RuntimeType::ERROR;
    }
}

int64_t Interpreter::handleFloatingPointOperation(RuntimeValue val1, RuntimeValue val2, OprCode oprCode) {
    uint32_t bit_pattern;
    float f1, f2, result;
    bit_pattern = static_cast<uint32_t>(val1.integer);
    memcpy(&f1, &bit_pattern, sizeof(float));
    bit_pattern = static_cast<uint32_t>(val2.integer);
    memcpy(&f2, &bit_pattern, sizeof(float));
    switch(oprCode) {
        case OprCode::ADD :
            result = f1 + f2;
            break;
        case OprCode::SUB : 
            result = f1 - f2;
            break;
        case OprCode::MUL : 
            result = f1 * f2;
            break;
        case OprCode::DIV : 
            result = f1 / f2;
            break;
        default:
            throw RuntimeError(
                "Runtime Error: Unsupported operation for Floating Point type"
            );
    }
    std::memcpy(&bit_pattern, &result, sizeof(result));
    return static_cast<int64_t>(bit_pattern);
}