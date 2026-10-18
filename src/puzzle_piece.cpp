#include "../include/puzzle_piece.h"
#include <cmath>

PuzzlePiece::PuzzlePiece(int id, int src_x, int src_y, int width, int height, int dest_x, int dest_y, int board_x, int board_y, int e_top, int e_right, int e_bottom, int e_left) {    
    this->id = id;
    this->is_locked = false;
    this->correct_x = src_x + board_x;
    this->correct_y = src_y + board_y;

    this->src_rect = {0, 0, width, height};
    this->dest_rect = {dest_x, dest_y, width, height};

    this->edges[0] = e_top;
    this->edges[1] = e_right;
    this->edges[2] = e_bottom;
    this->edges[3] = e_left;

    this->texture = nullptr; 
}

PuzzlePiece::~PuzzlePiece() {}

void PuzzlePiece::render(SDL_Renderer* renderer) {
    if (texture != nullptr) {
        SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
    }
}

void PuzzlePiece::set_position(int x, int y) {
    if (!is_locked) {
        dest_rect.x = x;
        dest_rect.y = y;
    }
}

void PuzzlePiece::set_texture(SDL_Texture* tex) {
    this->texture = tex;
}

void PuzzlePiece::free_texture() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

bool PuzzlePiece::is_clicked(int mouse_x, int mouse_y) {
    int pad_x = dest_rect.w / 8;
    int pad_y = dest_rect.h / 8;
    if (mouse_x >= (dest_rect.x + pad_x) && mouse_x <= (dest_rect.x + dest_rect.w - pad_x) &&
        mouse_y >= (dest_rect.y + pad_y) && mouse_y <= (dest_rect.y + dest_rect.h - pad_y)) {
        return true;
    }
    return false;
}

bool PuzzlePiece::check_snap(int threshold) {
    if (is_locked) return false;
    
    int dx = dest_rect.x - correct_x;
    int dy = dest_rect.y - correct_y;
    double distance = std::sqrt((dx * dx) + (dy * dy));

    if (distance <= threshold) {
        dest_rect.x = correct_x;
        dest_rect.y = correct_y;
        is_locked = true;
        return true; 
    }
    return false;
}

bool PuzzlePiece::get_is_locked() const { return is_locked; }
int PuzzlePiece::get_id() const { return id; }
int PuzzlePiece::get_width() const { return dest_rect.w; }
int PuzzlePiece::get_height() const { return dest_rect.h; }

int PuzzlePiece::get_edge(int side) const {
    if (side >= 0 && side < 4) {
        return edges[side];
    }
    return 0; 
}

int PuzzlePiece::get_x() const { return dest_rect.x; }
int PuzzlePiece::get_y() const { return dest_rect.y; }

void PuzzlePiece::set_locked_state(bool locked) {
    this->is_locked = locked;
    if (locked) {
        dest_rect.x = correct_x;
        dest_rect.y = correct_y;
    }
}