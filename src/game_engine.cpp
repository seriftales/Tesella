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
    locked_pieces_count = 0; 
    is_victory = false;
    board_offset_x = 150;
    board_offset_y = 100;
    board_w = 0;
    board_h = 0;
}

GameEngine::~GameEngine() {
    // Yıkıcı metotta değişken sıfırlanmaz, temizlik clean() içinde yapılır.
}

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

    int cols = 4; 
    int rows = 4; 

    int piece_w = main_surface->w / cols;
    int piece_h = main_surface->h / rows;

    board_offset_x = 150; 
    board_offset_y = 100; 

    // KRİTİK DÜZELTME: Başındaki 'int' kaldırıldı! Artık doğrudan sınıf üyesi olan board_w ve board_h güncelleniyor.
    // Çerçeve tam olarak yüklenen resmin (main_surface) boyutunda olacak.
    board_w = main_surface->w;
    board_h = main_surface->h;

    std::cout << "Resim Genişliği (Çerçeve): " << board_w << ", Resim Yüksekliği (Çerçeve): " << board_h << std::endl;
    std::cout << "Parça Genişliği: " << piece_w << ", Parça Yüksekliği: " << piece_h << std::endl;

    pieces.reserve(cols * rows);
    
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int id = row * cols + col;
            int src_x = col * piece_w;
            int src_y = row * piece_h;
            
            // Parçaları çerçevenin sağ tarafına dağıtıyoruz
            int dest_x = board_offset_x + board_w + 80 + (col * 40); 
            int dest_y = board_offset_y + (row * 40); 

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

            int overlap_x = piece_w / 6;
            int overlap_y = piece_h / 6;

            SDL_Rect crop_area = {src_x - overlap_x, src_y - overlap_y, piece_w + (overlap_x * 2), piece_h + (overlap_y * 2)};

            SDL_Surface* real_mask = SDL_CreateRGBSurfaceWithFormat(0, crop_area.w, crop_area.h, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_BlitScaled(format_mask, NULL, real_mask, NULL);
            SDL_FreeSurface(format_mask); 
            
            SDL_Texture* piece_texture = create_masked_texture(main_surface, real_mask, crop_area);
            SDL_FreeSurface(real_mask); 
            
            pieces.emplace_back(id, src_x - overlap_x, src_y - overlap_y, crop_area.w, crop_area.h, dest_x, dest_y, board_offset_x, board_offset_y, e_top, e_right, e_bottom, e_left);
            pieces.back().set_texture(piece_texture);
        }
    }

    SDL_FreeSurface(main_surface);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    locked_pieces_count = 0;
    is_victory = false;
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
                    if (!it->get_is_locked() && it->is_clicked(mouse_x, mouse_y)) {
                        selected_piece = &(*it); 
                        break; 
                    }
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                if (selected_piece != nullptr) {
                    // Sabit -100 yerine parçanın dinamik genişlik ve yüksekliğinin tam ortasından tutuyoruz
                    int center_offset_x = selected_piece->get_width() / 2;
                    int center_offset_y = selected_piece->get_height() / 2;
                    selected_piece->set_position(event.motion.x - center_offset_x, event.motion.y - center_offset_y);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                if (selected_piece != nullptr) {
                    // Parça yerine yaklaştığında (50 piksel tolerans) otomatik kilitlenir ve sayacı artırır
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

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Tesella Kontrol Paneli");
        if (is_victory) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "TEBRIKLER! YAPBOZ TAMAMLANDI!");
        } else {
            ImGui::Text("Kilitlenen Parca: %d / %d", locked_pieces_count, static_cast<int>(pieces.size()));
        }
        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Gri hedef çerçeve tam olarak yüklenen resmin boyutunda (board_w x board_h) çiziliyor
        SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255); 
        SDL_Rect board_rect = {board_offset_x, board_offset_y, board_w, board_h}; 
        SDL_RenderFillRect(renderer, &board_rect);
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255); 
        SDL_RenderDrawRect(renderer, &board_rect);

        // Önce kilitlenmiş parçaları alta çiz, kilitlenmemişleri üste çiz ki altta kalmasınlar
        for (auto& piece : pieces) {
            if (piece.get_is_locked()) {
                piece.render(renderer);
            }
        }
        for (auto& piece : pieces) {
            if (!piece.get_is_locked()) {
                piece.render(renderer);
            }
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