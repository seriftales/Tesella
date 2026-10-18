#ifndef PUZZLE_PIECE_H
#define PUZZLE_PIECE_H

#include <SDL2/SDL.h>

class PuzzlePiece {
public:
    PuzzlePiece(int id, int src_x, int src_y, int width, int height, int dest_x, int dest_y, int board_x, int board_y, int e_top, int e_right, int e_bottom, int e_left);
    ~PuzzlePiece();

    void render(SDL_Renderer* renderer);
    void set_position(int x, int y);
    void set_texture(SDL_Texture* tex);
    void free_texture();

    bool is_clicked(int mouse_x, int mouse_y);
    bool check_snap(int threshold = 50);

    bool get_is_locked() const;
    int get_id() const;
    int get_edge(int side) const;
    int get_width() const;
    int get_height() const;

private:
    int id;
    bool is_locked;
    int correct_x, correct_y;
    int edges[4]; 

    SDL_Rect src_rect;
    SDL_Rect dest_rect;
    SDL_Texture* texture;
};

#endif