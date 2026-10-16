#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    bool initialize(const char* title, int width, int height);
    void run();
    void clean();

private:
    bool is_running;
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* test_texture; 
};

#endif