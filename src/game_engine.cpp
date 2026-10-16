#include "../include/game_engine.h"
#include <iostream>

//Constructor
GameEngine::GameEngine() {
    window = nullptr;
    renderer = nullptr;
    is_running = false;
}

//Destructor
GameEngine::~GameEngine() {
}

// Yapılandırma
bool GameEngine::initialize(const char* title, int width, int height) {
    // SDL video alt sistemini başlat
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Başlatılamadı! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Pencere oluşturulamadı! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer oluşturulamadı! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    is_running = true;
    return true;
}

// Ana Motor
void GameEngine::run() {
    SDL_Event event;

    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false; 
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }
}

// Belleği Temizle
void GameEngine::clean() {

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Motor temizlendi ve kapatıldı." << std::endl;
}