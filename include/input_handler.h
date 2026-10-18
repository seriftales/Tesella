#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <SDL2/SDL.h>
#include <vector>
#include "puzzle_piece.h"

class InputHandler {
public:
    static void handle_mouse_event(
        const SDL_Event& event,
        std::vector<PuzzlePiece>& pieces,
        PuzzlePiece*& selected_piece,
        int& locked_pieces_count,
        bool& is_victory
    );
};

#endif