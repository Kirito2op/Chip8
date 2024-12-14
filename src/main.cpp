#include "chip8.h"
#include <exception>
#include <stdexcept>
#include <iostream>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

const int SCREEN_WIDTH = Chip8::VIDEO_WIDTH * 10;
const int SCREEN_HEIGHT = Chip8::VIDEO_HEIGHT * 10;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << '\n';
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Chip8 chip8;

    chip8.LoadROM("CONNECT4.ch8");

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                bool pressed = (e.type == SDL_KEYDOWN);
                int key = -1;

                switch (e.key.keysym.sym) {
                    case SDLK_1: key = 0x1; break;
                    case SDLK_2: key = 0x2; break;
                    case SDLK_3: key = 0x3; break;
                    case SDLK_4: key = 0xC; break;
                    case SDLK_q: key = 0x4; break;
                    case SDLK_w: key = 0x5; break;
                    case SDLK_e: key = 0x6; break;
                    case SDLK_r: key = 0xD; break;
                    case SDLK_a: key = 0x7; break;
                    case SDLK_s: key = 0x8; break;
                    case SDLK_d: key = 0x9; break;
                    case SDLK_f: key = 0xE; break;
                    case SDLK_z: key = 0xA; break;
                    case SDLK_x: key = 0x0; break;
                    case SDLK_c: key = 0xB; break;
                    case SDLK_v: key = 0xF; break;
                }

                if (key != -1) {
                    chip8.SetKeyState(key, pressed);
                }
            }
        }

        chip8.EmulateCycle();

        SDL_RenderClear(renderer);

        const auto& video = chip8.GetDisplay();
        for (int y = 0; y < Chip8::VIDEO_HEIGHT; ++y) {
            for (int x = 0; x < Chip8::VIDEO_WIDTH; ++x) {
                SDL_SetRenderDrawColor(renderer, video[y * Chip8::VIDEO_WIDTH + x] ? 255 : 0, 0, 0, 255); 
                SDL_Rect rect = { x * 10, y * 10, 10, 10 };  
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
