#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <array>
#include <stack>
#include <random>

class Chip8 {
public:
    static constexpr int VIDEO_WIDTH = 64;
    static constexpr int VIDEO_HEIGHT = 32;
    static constexpr int MEMORY_SIZE = 4096;
    static constexpr int NUM_REGISTERS = 16;
    static constexpr int STACK_SIZE = 16;
    static constexpr int NUM_KEYS = 16;

    Chip8();

    void LoadROM(const char* file);
    void EmulateCycle();
    const std::array<uint8_t, VIDEO_WIDTH*VIDEO_HEIGHT>& GetDisplay() const;
    bool DrawFlag() const;
    void SetKeyState(int key, bool pressed);

private:
    std::array<uint8_t, MEMORY_SIZE> memory;
    std::array<uint8_t, NUM_REGISTERS> V;
    uint16_t I;
    uint16_t pc;
    uint8_t delayTimer;
    uint8_t soundTimer;
    std::stack<uint16_t> stack;
    std::array<bool, NUM_KEYS> keypad;
    std::array<uint8_t, VIDEO_WIDTH*VIDEO_HEIGHT> video;

    uint16_t opcode;
    void ExecuteOpcode();

    std::mt19937 rng;
    std::uniform_int_distribution<uint8_t> randbyte;

    void OP_00E0();
    void OP_00EE();
    void OP_1NNN();
    void OP_2NNN();
    void OP_3XNN();
    void OP_4XNN();
    void OP_5XY0();
    void OP_6XNN();
    void OP_7XNN();
    void OP_8XY0();
    void OP_8XY1();
    void OP_8XY2();
    void OP_8XY3();
    void OP_8XY4();
    void OP_8XY5();
    void OP_8XY6();
    void OP_8XY7();
    void OP_8XYE();
    void OP_9XY0();
    void OP_ANNN();
    void OP_BNNN();
    void OP_CXNN();
    void OP_DXYN();
    void OP_EX9E();
    void OP_EXA1();
    void OP_FX07();
    void OP_FX0A();
    void OP_FX15();
    void OP_FX18();
    void OP_FX1E();
    void OP_FX29();
    void OP_FX33();
    void OP_FX55();
    void OP_FX65();
};

#endif