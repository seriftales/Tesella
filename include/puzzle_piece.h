#ifndef PUZZLE_PIECE_H
#define PUZZLE_PIECE_H

#include <SDL2/SDL.h>

class PuzzlePiece {
public:
    PuzzlePiece(int id, int src_x, int src_y, int width, int height, int dest_x, int dest_y);
    ~PuzzlePiece();

    void render(SDL_Renderer* renderer, SDL_Texture* texture);
    void set_position(int x, int y);
    
    bool is_clicked(int mouse_x, int mouse_y);
    void check_snap(int threshold = 20); // 20 piksellik bir çekim alanı (mıknatıs etkisi)

    bool get_is_locked() const;
    int get_id() const;

private:
    int id;
    bool is_locked;
    
    int correct_x;
    int correct_y;

    SDL_Rect src_rect;  
    SDL_Rect dest_rect; 
};

#endif