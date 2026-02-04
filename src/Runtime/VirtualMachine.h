#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

// Instruction set opcodes
enum Opcode {
    // Stack operations
    OP_PUSH,        // Push constant onto stack
    OP_POP,         // Pop value from stack

    // Arithmetic operations
    OP_ADD,         // Add top two values
    OP_SUB,         // Subtract top two values
    OP_MUL,         // Multiply top two values
    OP_DIV,         // Divide top two values
    OP_MOD,         // Modulo operation
    OP_NEG,         // Negate top value

    // Comparison operations
    OP_EQ,          // Equal
    OP_NE,          // Not equal
    OP_LT,          // Less than
    OP_LE,          // Less than or equal
    OP_GT,          // Greater than
    OP_GE,          // Greater than or equal

    // Logical operations
    OP_AND,         // Logical AND
    OP_OR,          // Logical OR
    OP_NOT,         // Logical NOT

    // Variable operations
    OP_LOAD,        // Load variable onto stack
    OP_STORE,       // Store top of stack to variable

    // Control flow
    OP_JMP,         // Unconditional jump
    OP_JMP_IF,      // Jump if top of stack is true
    OP_JMP_NOT,     // Jump if top of stack is false

    // Function operations
    OP_CALL,        // Call function
    OP_RET,         // Return from function

    // I/O operations
    OP_PRINT,       // Print top of stack
    OP_INPUT,       // Read input into variable

    // System
    OP_SLEEP,       // Sleep for specified seconds
    OP_HALT         // Stop execution
};

// Value types in the VM
enum class ValueType {
    INTEGER,
    BOOLEAN,
    NIL
};

// Runtime value
struct Value {
    ValueType type;
    int32_t intValue;

    Value();
    Value(int32_t val);
    Value(bool val);

    bool toBool() const;
    int32_t toInt() const;
    std::string toString() const;
};

// Call frame for function calls
struct CallFrame {
    size_t returnAddress;
    size_t framePointer;

    CallFrame(size_t ret, size_t fp);
};

class VirtualMachine {
private:
    std::vector<Value> stack;
    std::vector<uint8_t> code;
    std::unordered_map<std::string, Value> globals;
    std::vector<CallFrame> callStack;
    size_t ip;  // Instruction pointer
    size_t fp;  // Frame pointer

public:
    VirtualMachine();

    void load(const std::vector<uint8_t>& bytecode);
    void push(const Value& value);
    Value pop();
    uint8_t readByte();
    int32_t readInt32();
    std::string readString();
    void execute();
    void dumpStack();
    void dumpGlobals();
};

#endif

