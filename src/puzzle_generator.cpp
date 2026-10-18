#include "../include/puzzle_generator.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>

bool PuzzleGenerator::generate_pieces(
    SDL_Renderer* renderer,
    const std::string& image_path,
    int cols, int rows,
    int board_offset_x, int board_offset_y,
    int& out_board_w, int& out_board_h,
    SDL_Texture*& out_preview_texture,
    std::vector<PuzzlePiece>& out_pieces,
    const std::vector<std::array<int, 4>>* custom_edges
) {
    SDL_Surface* raw_surface = IMG_Load(image_path.c_str());
    if (!raw_surface) {
        std::cerr << "Resim yuklenemedi! Yol: " << image_path << std::endl;
        return false;
    }

    int target_board_w = 600;
    int target_board_h = 600;
    SDL_Surface* main_surface = SDL_CreateRGBSurfaceWithFormat(0, target_board_w, target_board_h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitScaled(raw_surface, NULL, main_surface, NULL);
    SDL_FreeSurface(raw_surface);

    if (out_preview_texture != nullptr) {
        SDL_DestroyTexture(out_preview_texture);
        out_preview_texture = nullptr;
    }
    out_preview_texture = SDL_CreateTextureFromSurface(renderer, main_surface);
    SDL_SetTextureBlendMode(out_preview_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(out_preview_texture, 65);

    srand(static_cast<unsigned int>(time(NULL)));

    int piece_w = main_surface->w / cols;
    int piece_h = main_surface->h / rows;

    out_board_w = main_surface->w;
    out_board_h = main_surface->h;

    out_pieces.clear();
    out_pieces.reserve(cols * rows);

    int spawn_start_x = board_offset_x + out_board_w + 50;
    int spawn_range_x = 250;
    int spawn_start_y = board_offset_y;
    int spawn_range_y = (out_board_h > piece_h) ? (out_board_h - piece_h) : 200;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int id = row * cols + col;
            int src_x = col * piece_w;
            int src_y = row * piece_h;

            int dest_x = spawn_start_x + (rand() % spawn_range_x);
            int dest_y = spawn_start_y + (rand() % spawn_range_y);

            int e_top = 0, e_right = 0, e_bottom = 0, e_left = 0;

            // EĞER KAYITTAN YÜKLENİYORSA KAYITLI KENARLARI KULLAN, YOKSA RASTGELE ÜRET
            if (custom_edges != nullptr && id < static_cast<int>(custom_edges->size())) {
                e_top    = (*custom_edges)[id][0];
                e_right  = (*custom_edges)[id][1];
                e_bottom = (*custom_edges)[id][2];
                e_left   = (*custom_edges)[id][3];
            } else {
                if (row > 0) {
                    int top_neighbor_index = (row - 1) * cols + col;
                    e_top = out_pieces[top_neighbor_index].get_edge(2) * -1;
                }
                if (col > 0) {
                    int left_neighbor_index = row * cols + (col - 1);
                    e_left = out_pieces[left_neighbor_index].get_edge(1) * -1;
                }
                e_right = (col < cols - 1) ? ((rand() % 2 == 0) ? 1 : -1) : 0;
                e_bottom = (row < rows - 1) ? ((rand() % 2 == 0) ? 1 : -1) : 0;
            }

            std::string mask_path = "assets/masks/mask_" +
                                    std::to_string(e_top) + "_" +
                                    std::to_string(e_right) + "_" +
                                    std::to_string(e_bottom) + "_" +
                                    std::to_string(e_left) + ".png";

            SDL_Surface* temp_mask = IMG_Load(mask_path.c_str());
            if (!temp_mask) {
                SDL_FreeSurface(main_surface);
                return false;
            }

            SDL_Surface* format_mask = SDL_ConvertSurfaceFormat(temp_mask, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(temp_mask);

            int overlap_x = piece_w / 6;
            int overlap_y = piece_h / 6;

            SDL_Rect crop_area = {src_x - overlap_x, src_y - overlap_y, piece_w + (overlap_x * 2), piece_h + (overlap_y * 2)};

            SDL_Surface* real_mask = SDL_CreateRGBSurfaceWithFormat(0, crop_area.w, crop_area.h, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_BlitScaled(format_mask, NULL, real_mask, NULL);
            SDL_FreeSurface(format_mask);

            SDL_Texture* piece_texture = create_masked_texture(renderer, main_surface, real_mask, crop_area);
            SDL_FreeSurface(real_mask);

            out_pieces.emplace_back(id, src_x - overlap_x, src_y - overlap_y, crop_area.w, crop_area.h, dest_x, dest_y, board_offset_x, board_offset_y, e_top, e_right, e_bottom, e_left);
            out_pieces.back().set_texture(piece_texture);
        }
    }

    SDL_FreeSurface(main_surface);

    // Sadece yeni oyunda rastgele karıştır, kayıttan yüklenirken sırayı bozma
    if (custom_edges == nullptr) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(out_pieces.begin(), out_pieces.end(), g);
    }

    return true;
}

SDL_Texture* PuzzleGenerator::create_masked_texture(SDL_Renderer* renderer, SDL_Surface* original_surface, SDL_Surface* mask_surface, SDL_Rect crop_area) {
    SDL_Surface* masked_surface = SDL_CreateRGBSurfaceWithFormat(0, crop_area.w, crop_area.h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(masked_surface, NULL, SDL_MapRGBA(masked_surface->format, 0, 0, 0, 0));

    SDL_Rect src_rect = crop_area;
    SDL_Rect dst_rect = {0, 0, crop_area.w, crop_area.h};

    if (src_rect.x < 0) {
        dst_rect.x = -src_rect.x;
        src_rect.w += src_rect.x;
        src_rect.x = 0;
    }
    if (src_rect.y < 0) {
        dst_rect.y = -src_rect.y;
        src_rect.h += src_rect.y;
        src_rect.y = 0;
    }

    SDL_BlitSurface(original_surface, &src_rect, masked_surface, &dst_rect);

    SDL_LockSurface(masked_surface);
    SDL_LockSurface(mask_surface);

    for (int y = 0; y < crop_area.h; ++y) {
        Uint32* dest_row = (Uint32*)((Uint8*)masked_surface->pixels + y * masked_surface->pitch);
        Uint32* mask_row = (Uint32*)((Uint8*)mask_surface->pixels + y * mask_surface->pitch);

        for (int x = 0; x < crop_area.w; ++x) {
            Uint8 r, g, b, a;
            SDL_GetRGBA(mask_row[x], mask_surface->format, &r, &g, &b, &a);

            if (r < 10 && g < 10 && b < 10) {
                Uint8 pr, pg, pb, pa;
                SDL_GetRGBA(dest_row[x], masked_surface->format, &pr, &pg, &pb, &pa);
                dest_row[x] = SDL_MapRGBA(masked_surface->format, pr, pg, pb, 0);
            }
        }
    }

    SDL_UnlockSurface(mask_surface);
    SDL_UnlockSurface(masked_surface);

    SDL_Texture* final_texture = SDL_CreateTextureFromSurface(renderer, masked_surface);
    SDL_SetTextureBlendMode(final_texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(masked_surface);

    return final_texture;
}