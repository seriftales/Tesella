#include "../include/input_handler.h"
#include <iostream>
#include <algorithm>

void InputHandler::handle_mouse_event(
    const SDL_Event& event,
    std::vector<PuzzlePiece>& pieces,
    PuzzlePiece*& selected_piece,
    int& locked_pieces_count,
    bool& is_victory,
    int window_w,
    int window_h,
    bool& out_piece_snapped
) {
    static int drag_offset_x = 0;
    static int drag_offset_y = 0;

    out_piece_snapped = false;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouse_x = event.button.x;
        int mouse_y = event.button.y;

        for (int i = static_cast<int>(pieces.size()) - 1; i >= 0; --i) {
            if (!pieces[i].get_is_locked() && pieces[i].is_clicked(mouse_x, mouse_y)) {
                PuzzlePiece clicked_piece = pieces[i];
                pieces.erase(pieces.begin() + i);
                pieces.push_back(clicked_piece);

                selected_piece = &pieces.back();

                drag_offset_x = mouse_x - selected_piece->get_x();
                drag_offset_y = mouse_y - selected_piece->get_y();
                break;
            }
        }
    }
    else if (event.type == SDL_MOUSEMOTION) {
        if (selected_piece != nullptr) {
            int new_x = event.motion.x - drag_offset_x;
            int new_y = event.motion.y - drag_offset_y;

            int max_x = std::max(0, window_w - selected_piece->get_width());
            int max_y = std::max(0, window_h - selected_piece->get_height());

            new_x = std::clamp(new_x, 0, max_x);
            new_y = std::clamp(new_y, 0, max_y);

            selected_piece->set_position(new_x, new_y);
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (selected_piece != nullptr) {
            if (selected_piece->check_snap(50)) {
                locked_pieces_count++;
                out_piece_snapped = true; 
                if (locked_pieces_count >= static_cast<int>(pieces.size())) {
                    is_victory = true;
                }
            }
            selected_piece = nullptr;
        }
    }
}