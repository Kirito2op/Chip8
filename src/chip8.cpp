#include "chip8.h"
#include <exception>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstring>

Chip8::Chip8()
    : pc(0x200), I(0), delayTimer(0), soundTimer(0), opcode(0), 
      rng(std::random_device{}()), randbyte(0, 255) {
    
    memory.fill(0);
    video.fill(0);
    V.fill(0);
    keypad.fill(false);

    const uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    memcpy(memory.data(), fontset, sizeof(fontset));
}

void Chip8::LoadROM(const char* filename){
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open()){
        std::cerr << "Failed to open ROM" << '\n';
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    file.read(reinterpret_cast<char*> (&memory[0x200]), size);
}

void Chip8::EmulateCycle(){
    opcode = memory[pc] << 8 | memory[pc + 1];

    ExecuteOpcode();

    if(delayTimer > 0) delayTimer--;

    if(soundTimer > 0) soundTimer--;
}

const std::array<uint8_t, Chip8::VIDEO_WIDTH * Chip8::VIDEO_HEIGHT>& Chip8::GetDisplay() const {
    return video;
}

bool Chip8::DrawFlag() const{
    return true;
}

void Chip8::SetKeyState(int key, bool pressed){
    if(key>=0 && key < NUM_KEYS){
        keypad[key] = pressed;
    }
}

void Chip8::OP_00E0() {
    video.fill(0);
}

void Chip8::OP_1NNN() {
    pc = opcode & 0x0FFF;
}

void Chip8::OP_00EE() {
    pc = stack.top();
    stack.pop();
    pc += 2;
}

void Chip8::OP_2NNN() {
    stack.push(pc);
    pc = opcode & 0x0FFF;
}

void Chip8::OP_3XNN() {
    if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_4XNN() {
    if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_5XY0() {
    if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]){
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_6XNN() {
    V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
    pc += 2;
}

void Chip8::OP_7XNN() {
    V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
    pc += 2;
}

void Chip8::OP_8XY0() {
    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

void Chip8::OP_8XY1() {
    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

void Chip8::OP_8XY2() {
    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

void Chip8::OP_8XY3() {
    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

void Chip8::OP_8XY4() {
    if(V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4])) {
        V[0xF] = 1;
    }
    else{
        V[0xF] = 0;
    }
    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

void Chip8::OP_8XY5() {
    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
        V[0xF] = 0;
    }
    else{
        V[0xF] = 1;
    }
    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

void Chip8::OP_8XY6() {
    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
    V[(opcode & 0x0F00) >> 8] >>= 1;
    pc += 2;
}

void Chip8::OP_8XY7() {
    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
        V[0xF] = 0;
    else
        V[0xF] = 1;
    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
    pc += 2;
}

void Chip8::OP_8XYE() {
    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
    V[(opcode & 0x0F00) >> 8] <<= 1;
    pc += 2;
}

void Chip8::OP_9XY0() {
    if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_ANNN() {
    I = opcode & 0x0FFF;
    pc += 2;
}

void Chip8::OP_BNNN() {
    pc = (opcode & 0x0FFF) + V[0];
}

void Chip8::OP_CXNN() {
    V[(opcode & 0x0F00) >> 8] = (randbyte(rng) & (opcode & 0x00FF));
    pc += 2;
}

void Chip8::OP_EX9E() {
    if(keypad[V[(opcode & 0x0F00) >> 8]] != 0) {
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_EXA1() {
    if(keypad[V[(opcode & 0x0F00) >> 8]] == 0) {
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_FX07() {
    V[(opcode & 0x0F00) >> 8] = delayTimer;
    pc += 2;
}

void Chip8::OP_FX0A() {
    bool key_pressed = false;

    for(int i = 0; i < 16; i++) {
        if(keypad[i] != 0){
            V[(opcode & 0x0F00) >> 8] = i;
            key_pressed = true;
        }
    }

    if(!key_pressed){
        return;
    }
    pc += 2;
}

void Chip8::OP_FX15() {
    delayTimer = V[(opcode & 0x0F00) >> 8];
    pc += 2;
}

void Chip8::OP_FX18() {
    soundTimer = V[(opcode & 0x0F00) >> 8];
    pc += 2;
}

void Chip8::OP_FX1E() {
     if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
        V[0xF] = 1;
    else
        V[0xF] = 0;
    I += V[(opcode & 0x0F00) >> 8];
    pc += 2;
}

void Chip8::OP_FX29() {
    I = V[(opcode & 0x0F00) >> 8] * 0x5;
    pc += 2;
}

void Chip8::OP_FX33() {
    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
    memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
    pc += 2;
}

void Chip8::OP_FX55() {
    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
        memory[I + i] = V[i];
    }

    I += ((opcode & 0x0F00) >> 8) + 1;
    pc += 2;
}

void Chip8::OP_FX65() {
    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
        V[i] = memory[I + i];
    }

    I += ((opcode & 0x0F00) >> 8) + 1;
    pc += 2;
}

void Chip8::OP_DXYN() {
    unsigned short x = V[(opcode & 0x0F00) >> 8]; 
    unsigned short y = V[(opcode & 0x00F0) >> 4]; 
    unsigned short n = opcode & 0x000F;            
    
    V[0xF] = 0; 

    for (unsigned short row = 0; row < n; ++row) {
        uint8_t spriteRow = memory[I + row]; 
        for (unsigned short col = 0; col < 8; ++col) {
            if ((spriteRow & (0x80 >> col)) != 0) { 
                unsigned short px = (x + col) % 64; 
                unsigned short py = (y + row) % 32; 

                if (video[py * 64 + px] == 1) {
                    V[0xF] = 1; 
                }

                video[py * 64 + px] ^= 1; 
            }
        }
    }

    pc += 2;
}

void Chip8::ExecuteOpcode() {
    switch(opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: OP_00E0(); break;
                case 0x00EE: OP_00EE(); break;
            }
            break;
        case 0x1000: OP_1NNN(); break;
        case 0x2000: OP_2NNN(); break;
        case 0x3000: OP_3XNN(); break;
        case 0x4000: OP_4XNN(); break;
        case 0x5000: OP_5XY0(); break;
        case 0x6000: OP_6XNN(); break;
        case 0x7000: OP_7XNN(); break;
        case 0x8000: 
            switch (opcode & 0x000F) {
                case 0x0000: OP_8XY0(); break;
                case 0x0001: OP_8XY1(); break;
                case 0x0002: OP_8XY2(); break;
                case 0x0003: OP_8XY3(); break;
                case 0x0004: OP_8XY4(); break;
                case 0x0005: OP_8XY5(); break;
                case 0x0006: OP_8XY6(); break;
                case 0x0007: OP_8XY7(); break;
                case 0x000E: OP_8XYE(); break;
            }
            break;
        case 0x9000: OP_9XY0(); break;
        case 0xA000: OP_ANNN(); break;
        case 0xB000: OP_BNNN(); break;
        case 0xC000: OP_CXNN(); break;
        case 0xD000: OP_DXYN(); break;
        case 0xE000: 
            switch (opcode & 0x00FF) {
                case 0x009E: OP_EX9E(); break;
                case 0x00A1: OP_EXA1(); break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: OP_FX07(); break;
                case 0x000A: OP_FX0A(); break;
                case 0x0015: OP_FX15(); break;
                case 0x001E: OP_FX1E(); break;
                case 0x0029: OP_FX29(); break;
                case 0x0033: OP_FX33(); break;
                case 0x0055: OP_FX55(); break;
                case 0x0065: OP_FX65(); break;
            }
        break;
    }
}

