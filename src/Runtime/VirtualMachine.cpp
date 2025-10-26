#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>

#include "VirtualMachine.h"

Value::Value() : type(ValueType::NIL), intValue(0) {}

Value::Value(int32_t val) : type(ValueType::INTEGER), intValue(val) {}

Value::Value(bool val) : type(ValueType::BOOLEAN), intValue(val ? 1 : 0) {}

bool Value::toBool() const {
    if (type == ValueType::BOOLEAN) return intValue != 0;
    if (type == ValueType::INTEGER) return intValue != 0;
    return false;
}

int32_t Value::toInt() const {
    return intValue;
}

std::string Value::toString() const {
    if (type == ValueType::BOOLEAN) return intValue ? "true" : "false";
    if (type == ValueType::INTEGER) return std::to_string(intValue);
    return "nil";
}

CallFrame::CallFrame(size_t ret, size_t fp) : returnAddress(ret), framePointer(fp) {}

VirtualMachine::VirtualMachine() : ip(0), fp(0) {}

void VirtualMachine::load(const std::vector<uint8_t>& bytecode) {
    code = bytecode;
    ip = 0;
    fp = 0;
    stack.clear();
    globals.clear();
    callStack.clear();
}

void VirtualMachine::push(const Value& value) {
    stack.push_back(value);
}

Value VirtualMachine::pop() {
    if (stack.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value value = stack.back();
    stack.pop_back();
    return value;
}

uint8_t VirtualMachine::readByte() {
    if (ip >= code.size()) {
        throw std::runtime_error("Instruction pointer out of bounds");
    }
    return code[ip++];
}

int32_t VirtualMachine::readInt32() {
    int32_t value = 0;
    value |= readByte();
    value |= readByte() << 8;
    value |= readByte() << 16;
    value |= readByte() << 24;
    return value;
}

std::string VirtualMachine::readString() {
    uint8_t length = readByte();
    std::string str;
    for (uint8_t i = 0; i < length; i++) {
        str += static_cast<char>(readByte());
    }
    return str;
}

void VirtualMachine::execute() {
    bool running = true;

    while (running && ip < code.size()) {
        uint8_t opcode = readByte();

        switch (opcode) {
            case OP_PUSH: {
                int32_t value = readInt32();
                push(Value(value));
                break;
            }

            case OP_POP:
                pop();
                break;

            case OP_ADD: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() + b.toInt()));
                break;
            }

            case OP_SUB: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() - b.toInt()));
                break;
            }

            case OP_MUL: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() * b.toInt()));
                break;
            }

            case OP_DIV: {
                Value b = pop();
                Value a = pop();
                if (b.toInt() == 0) {
                    throw std::runtime_error("Division by zero");
                }
                push(Value(a.toInt() / b.toInt()));
                break;
            }

            case OP_MOD: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() % b.toInt()));
                break;
            }

            case OP_NEG: {
                Value a = pop();
                push(Value(-a.toInt()));
                break;
            }

            case OP_EQ: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() == b.toInt()));
                break;
            }

            case OP_NE: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() != b.toInt()));
                break;
            }

            case OP_LT: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() < b.toInt()));
                break;
            }

            case OP_LE: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() <= b.toInt()));
                break;
            }

            case OP_GT: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() > b.toInt()));
                break;
            }

            case OP_GE: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toInt() >= b.toInt()));
                break;
            }

            case OP_AND: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toBool() && b.toBool()));
                break;
            }

            case OP_OR: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toBool() || b.toBool()));
                break;
            }

            case OP_NOT: {
                Value a = pop();
                push(Value(!a.toBool()));
                break;
            }

            case OP_LOAD: {
                std::string varName = readString();
                if (globals.find(varName) == globals.end()) {
                    throw std::runtime_error("Undefined variable: " + varName);
                }
                push(globals[varName]);
                break;
            }

            case OP_STORE: {
                std::string varName = readString();
                Value value = pop();
                globals[varName] = value;
                break;
            }

            case OP_JMP: {
                int32_t offset = readInt32();
                ip = offset;
                break;
            }

            case OP_JMP_IF: {
                int32_t offset = readInt32();
                Value condition = pop();
                if (condition.toBool()) {
                    ip = offset;
                }
                break;
            }

            case OP_JMP_NOT: {
                int32_t offset = readInt32();
                Value condition = pop();
                if (!condition.toBool()) {
                    ip = offset;
                }
                break;
            }

            case OP_CALL: {
                int32_t funcAddr = readInt32();
                callStack.emplace_back(ip, fp);
                fp = stack.size();
                ip = funcAddr;
                break;
            }

            case OP_RET: {
                if (callStack.empty()) {
                    throw std::runtime_error("Return outside function");
                }
                CallFrame frame = callStack.back();
                callStack.pop_back();

                Value returnValue;
                if (!stack.empty() && stack.size() > frame.framePointer) {
                    returnValue = pop();
                }

                while (stack.size() > frame.framePointer) {
                    pop();
                }

                if (returnValue.type != ValueType::NIL) {
                    push(returnValue);
                }

                ip = frame.returnAddress;
                fp = frame.framePointer;
                break;
            }

            case OP_PRINT:
                std::cout << stack.back().toString() << std::endl;
                break;

            case OP_INPUT: {
                std::string varName = readString();
                int32_t value;
                std::cin >> value;
                globals[varName] = Value(value);
                break;
            }

            case OP_HALT:
                running = false;
                break;

            default:
                throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
        }
    }
}

void VirtualMachine::dumpStack() {
    std::cout << "Stack: [";
    for (size_t i = 0; i < stack.size(); i++) {
        std::cout << stack[i].toString();
        if (i < stack.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void VirtualMachine::dumpGlobals() {
    std::cout << "Globals: {";
    bool first = true;
    for (const auto& pair : globals) {
        if (!first) std::cout << ", ";
        std::cout << pair.first << ": " << pair.second.toString();
        first = false;
    }
    std::cout << "}" << std::endl;
}
