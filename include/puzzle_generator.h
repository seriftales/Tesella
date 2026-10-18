#ifndef PUZZLE_GENERATOR_H
#define PUZZLE_GENERATOR_H

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include "puzzle_piece.h"

class PuzzleGenerator {
public:
    static bool generate_pieces(
        SDL_Renderer* renderer,
        const std::string& image_path,
        int cols, int rows,
        int board_offset_x, int board_offset_y,
        int& out_board_w, int& out_board_h,
        std::vector<PuzzlePiece>& out_pieces
    );

private:
    static SDL_Texture* create_masked_texture(
        SDL_Renderer* renderer,
        SDL_Surface* original_surface,
        SDL_Surface* mask_surface,
        SDL_Rect crop_area
    );
};

#endif