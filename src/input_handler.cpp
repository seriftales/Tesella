#include "../include/input_handler.h"
#include <iostream>
#include <algorithm>

void InputHandler::handle_mouse_event(
    const SDL_Event& event,
    std::vector<PuzzlePiece>& pieces,
    PuzzlePiece*& selected_piece,
    int& locked_pieces_count,
    bool& is_victory
) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouse_x = event.button.x;
        int mouse_y = event.button.y;

        // Vektörü sondan başa (en üstteki parçadan en alttakine) doğru tara
        for (int i = static_cast<int>(pieces.size()) - 1; i >= 0; --i) {
            if (!pieces[i].get_is_locked() && pieces[i].is_clicked(mouse_x, mouse_y)) {
                // Z-INDEX (BRING TO FRONT): Tıklanan parçayı vektörün en sonuna (en üst katmana) taşı!
                PuzzlePiece clicked_piece = pieces[i];
                pieces.erase(pieces.begin() + i);
                pieces.push_back(clicked_piece);

                // Artık en sonda olduğu için adresini güvenle seçili parça yapabiliriz
                selected_piece = &pieces.back();
                break;
            }
        }
    }
    else if (event.type == SDL_MOUSEMOTION) {
        if (selected_piece != nullptr) {
            int center_offset_x = selected_piece->get_width() / 2;
            int center_offset_y = selected_piece->get_height() / 2;
            selected_piece->set_position(event.motion.x - center_offset_x, event.motion.y - center_offset_y);
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (selected_piece != nullptr) {
            if (selected_piece->check_snap(50)) {
                locked_pieces_count++;
                std::cout << "Parca kilitlendi! Toplam: " << locked_pieces_count << " / " << pieces.size() << std::endl;
                if (locked_pieces_count >= static_cast<int>(pieces.size())) {
                    is_victory = true;
                }
            }
            selected_piece = nullptr;
        }
    }
}