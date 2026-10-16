#ifndef PUZZLE_PIECE_H
#define PUZZLE_PIECE_H

#include <SDL2/SDL.h>

class PuzzlePiece {
public:
    PuzzlePiece(int id, int src_x, int src_y, int width, int height, int dest_x, int dest_y, int e_top, int e_right, int e_bottom, int e_left);
    ~PuzzlePiece();

    void render(SDL_Renderer* renderer);
    void set_position(int x, int y);
    bool is_clicked(int mouse_x, int mouse_y);
    void check_snap(int threshold = 20); 

    void set_texture(SDL_Texture* tex);
    void free_texture() ; 

    bool get_is_locked() const;
    int get_id() const;
    int get_edge(int side) const;

private:
    int id;
    bool is_locked;
    int correct_x;
    int correct_y;
    int edges[4];

    SDL_Rect src_rect;  
    SDL_Rect dest_rect; 
    SDL_Texture* texture;

};

#endif