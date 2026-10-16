#include "../include/game_engine.h"
#include <iostream>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <ctime>
#include <string> 

GameEngine::GameEngine() {
    window = nullptr;
    renderer = nullptr;
    test_texture = nullptr;
    selected_piece = nullptr; 
    is_running = false;
}

GameEngine::~GameEngine() {}

bool GameEngine::initialize(const char* title, int width, int height) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL başlatılamadı! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image başlatılamadı! Hata: " << IMG_GetError() << std::endl;
        return false;
    }

    SDL_Surface* main_surface = IMG_Load("assets/images/debug_grid.png");
    if (!main_surface) return false;

    srand(static_cast<unsigned int>(time(NULL)));

    int cols = 4 ; 
    int rows = 4 ; 

    int piece_w = main_surface->w / cols;
    int piece_h = main_surface->h / rows;

    pieces.reserve(16) ; 

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int id = row * cols + col;
            int src_x = col * piece_w;
            int src_y = row * piece_h;
            int dest_x = 800 + (col * 30); 
            int dest_y = 100 + (row * 30); 

            int e_top = 0; 
            if (row > 0) {
                int top_neighbor_index = (row - 1) * cols + col;
                e_top = pieces[top_neighbor_index].get_edge(2) * -1;
            }

            int e_left = 0;
            if (col > 0) {
                int left_neighbor_index = row * cols + (col - 1);
                e_left = pieces[left_neighbor_index].get_edge(1) * -1;
            }

           
            int e_right = 0;
            if (col < cols - 1) {
                e_right = (rand() % 2 == 0) ? 1 : -1;
            }

            int e_bottom = 0;
            if (row < rows - 1) {
                e_bottom = (rand() % 2 == 0) ? 1 : -1;
            }

            std::string mask_path = "assets/masks/mask_" + 
                                    std::to_string(e_top) + "_" + 
                                    std::to_string(e_right) + "_" + 
                                    std::to_string(e_bottom) + "_" + 
                                    std::to_string(e_left) + ".png";
            
            SDL_Surface* temp_mask = IMG_Load(mask_path.c_str());
            if (!temp_mask) {
                std::cerr << "Kritik Hata: Maske dosyasi bulunamadi! Yol: " << mask_path << std::endl;
                return false; 
            }

            SDL_Surface* format_mask = SDL_ConvertSurfaceFormat(temp_mask, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(temp_mask);

            int overlap_x = piece_w / 6 ;
            int overlap_y = piece_h / 6 ;


            SDL_Rect crop_area = {src_x - overlap_x, src_y - overlap_y, piece_w + (overlap_x * 2), piece_h + (overlap_y * 2)};

            SDL_Surface* real_mask = SDL_CreateRGBSurfaceWithFormat(0, crop_area.w, crop_area.h, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_BlitScaled(format_mask, NULL, real_mask, NULL);
            SDL_FreeSurface(format_mask); 
            
            SDL_Texture* piece_texture = create_masked_texture(main_surface, real_mask, crop_area);
            SDL_FreeSurface(real_mask); 
            
            pieces.emplace_back(id, src_x - overlap_x, src_y - overlap_y, crop_area.w, crop_area.h, dest_x, dest_y, e_top, e_right, e_bottom, e_left);
            pieces.back().set_texture(piece_texture);

            std::cout << "Parça ID: " << id 
                      << " | Üst: " << e_top 
                      << " | Sağ: " << e_right 
                      << " | Alt: " << e_bottom 
                      << " | Sol: " << e_left << std::endl;
        
        
        }
    }

    SDL_FreeSurface(main_surface);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    is_running = true;
    return true;
    
}

void GameEngine::run() {
    SDL_Event event;

    while (is_running) { 
        
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                is_running = false;
            }

            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse) continue;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouse_x = event.button.x;
                int mouse_y = event.button.y;

                for (auto it = pieces.rbegin(); it != pieces.rend(); ++it) {
                    if (it->is_clicked(mouse_x, mouse_y) && !it->get_is_locked()) {
                        selected_piece = &(*it); 
                        break; 
                    }
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                if (selected_piece != nullptr) {
                    selected_piece->set_position(event.motion.x - 75, event.motion.y - 75);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                if (selected_piece != nullptr) {
                    selected_piece->check_snap(30); 
                    selected_piece = nullptr; 
                }
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Tesella Kontrol Paneli");
        ImGui::Text("Gorsel parcalandi ve GPU ile ciziliyor.");
        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (auto& piece : pieces) {
            piece.render(renderer);
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        
    } 
}

void GameEngine::clean() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    for (auto& piece : pieces) {
        piece.free_texture();
    }
    
    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Motor temizlendi ve kapatıldı." << std::endl;
}

SDL_Texture* GameEngine::create_masked_texture(SDL_Surface* original_surface, SDL_Surface* mask_surface, SDL_Rect crop_area) {
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