#include "../include/puzzle_piece.h"
#include <cmath>

PuzzlePiece::PuzzlePiece(int id, int src_x, int src_y, int width, int height, int dest_x, int dest_y, int e_top, int e_right, int e_bottom, int e_left) {
    this->id = id;
    this->is_locked = false;
    this->correct_x = src_x;
    this->correct_y = src_y;
    this->src_rect = {src_x, src_y, width, height};
    this->dest_rect = {dest_x, dest_y, width, height};

    // Kenar durumlarını sınıfın içindeki diziye kaydet
    this->edges[0] = e_top;
    this->edges[1] = e_right;
    this->edges[2] = e_bottom;
    this->edges[3] = e_left;
}

PuzzlePiece::~PuzzlePiece() {}

void PuzzlePiece::render(SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
}

void PuzzlePiece::set_position(int x, int y) {
    if (!is_locked) {
        dest_rect.x = x;
        dest_rect.y = y;
    }
}

bool PuzzlePiece::is_clicked(int mouse_x, int mouse_y) {
    if (mouse_x >= dest_rect.x && mouse_x <= (dest_rect.x + dest_rect.w) &&
        mouse_y >= dest_rect.y && mouse_y <= (dest_rect.y + dest_rect.h)) {
        return true;
    }
    return false;
}

void PuzzlePiece::check_snap(int threshold) {
    if (is_locked) return;
    int dx = dest_rect.x - correct_x;
    int dy = dest_rect.y - correct_y;
    double distance = std::sqrt((dx * dx) + (dy * dy));

    if (distance <= threshold) {
        dest_rect.x = correct_x;
        dest_rect.y = correct_y;
        is_locked = true;
    }
}

bool PuzzlePiece::get_is_locked() const { return is_locked; }
int PuzzlePiece::get_id() const { return id; }

int PuzzlePiece::get_edge(int side) const {
    if (side >= 0 && side < 4) {
        return edges[side];
    }
    return 0; 
}