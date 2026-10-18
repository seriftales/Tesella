#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "puzzle_piece.h"
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
    bool is_running = false;
    int locked_pieces_count = 0; 
    bool is_victory = false;         

    int board_offset_x = 150;
    int board_offset_y = 100;
    int board_w = 0;
    int board_h = 0;
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* test_texture = nullptr; 
    SDL_Texture* create_masked_texture(SDL_Surface* original_surface, SDL_Surface* mask_surface, SDL_Rect crop_area);

    std::vector<PuzzlePiece> pieces;
    PuzzlePiece* selected_piece = nullptr;
};

#endif