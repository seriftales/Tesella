#include "../include/game_engine.h"
#include "../include/puzzle_generator.h"
#include "../include/input_handler.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <array>
#include <SDL2/SDL_image.h>

GameEngine::GameEngine() {}
GameEngine::~GameEngine() {}

std::string GameEngine::get_save_path() const {
    std::string uname(username_buffer);
    if (uname.empty()) uname = "default";
    return "saves/" + uname + ".sav";
}

bool GameEngine::has_save_file() const {
    std::ifstream file(get_save_path());
    return file.good();
}

void GameEngine::scan_images_directory() {
    available_images.clear();
    std::string dir_path = "assets/images/puzzle";
    std::filesystem::create_directories(dir_path);

    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                available_images.push_back(entry.path().string());
            }
        }
    }

    std::sort(available_images.begin(), available_images.end());

    if (!available_images.empty()) {
        selected_image_index = 0;
        current_image_path = available_images[0];
        update_menu_preview(0);
    } else {
        status_message = "UYARI: assets/images/puzzle klasorunde hic resim bulunamadi!";
    }
}

void GameEngine::update_menu_preview(int index) {
    if (index < 0 || index >= static_cast<int>(available_images.size())) return;

    if (menu_preview_texture != nullptr) {
        SDL_DestroyTexture(menu_preview_texture);
        menu_preview_texture = nullptr;
    }

    SDL_Surface* surf = IMG_Load(available_images[index].c_str());
    if (surf) {
        menu_preview_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

void GameEngine::start_new_game() {
    if (available_images.empty()) {
        status_message = "HATA: Oynanacak resim secilmedi!";
        return;
    }

    current_image_path = available_images[selected_image_index];

    for (auto& piece : pieces) {
        piece.free_texture();
    }
    selected_piece = nullptr;
    locked_pieces_count = 0;
    is_victory = false;
    saved_elapsed_seconds = 0.0f;
    elapsed_seconds = 0.0f;
    start_ticks = SDL_GetTicks();

    PuzzleGenerator::generate_pieces(
        renderer, current_image_path,
        grid_size, grid_size,
        board_offset_x, board_offset_y,
        board_w, board_h,
        preview_texture, pieces,
        nullptr
    );
    current_state = GameState::PLAYING;
    status_message = "Yeni oyun baslatildi: " + current_image_path;
}

bool GameEngine::save_game() {
    std::filesystem::create_directories("saves");
    std::ofstream file(get_save_path());
    if (!file.is_open()) return false;

    file << current_image_path << "\n";
    file << grid_size << " " << locked_pieces_count << " " << is_victory << " " << elapsed_seconds << "\n";
    file << pieces.size() << "\n";

    for (const auto& p : pieces) {
        file << p.get_id() << " "
             << p.get_x() << " "
             << p.get_y() << " "
             << p.get_is_locked() << " "
             << p.get_edge(0) << " "
             << p.get_edge(1) << " "
             << p.get_edge(2) << " "
             << p.get_edge(3) << "\n";
    }

    status_message = "Oyun kaydedildi: " + get_save_path();
    return true;
}

bool GameEngine::load_game() {
    std::ifstream file(get_save_path());
    if (!file.is_open()) return false;

    std::getline(file >> std::ws, current_image_path);
    file >> grid_size >> locked_pieces_count >> is_victory >> saved_elapsed_seconds;

    size_t count = 0;
    file >> count;
    if (count != static_cast<size_t>(grid_size * grid_size)) return false;

    struct SavedPieceData {
        int id, x, y;
        bool locked;
        std::array<int, 4> edges;
    };

    std::vector<SavedPieceData> saved_order(count);
    std::vector<std::array<int, 4>> custom_edges(count);

    for (size_t i = 0; i < count; ++i) {
        SavedPieceData d;
        file >> d.id >> d.x >> d.y >> d.locked
             >> d.edges[0] >> d.edges[1] >> d.edges[2] >> d.edges[3];
        saved_order[i] = d;
        if (d.id >= 0 && d.id < static_cast<int>(count)) {
            custom_edges[d.id] = d.edges;
        }
    }

    for (auto& piece : pieces) {
        piece.free_texture();
    }
    selected_piece = nullptr;

    if (!PuzzleGenerator::generate_pieces(
        renderer, current_image_path,
        grid_size, grid_size,
        board_offset_x, board_offset_y,
        board_w, board_h,
        preview_texture, pieces,
        &custom_edges
    )) {
        status_message = "HATA: Kayitli oyunun resmi bulunamadi: " + current_image_path;
        return false;
    }

    std::vector<PuzzlePiece> ordered_pieces;
    ordered_pieces.reserve(count);

    for (const auto& d : saved_order) {
        for (auto& p : pieces) {
            if (p.get_id() == d.id) {
                p.set_position(d.x, d.y);
                p.set_locked_state(d.locked);
                ordered_pieces.push_back(p);
                break;
            }
        }
    }

    pieces = std::move(ordered_pieces);
    start_ticks = SDL_GetTicks();
    elapsed_seconds = saved_elapsed_seconds;
    current_state = GameState::PLAYING;
    status_message = "Kayitli oyun yuklendi!";
    return true;
}

bool GameEngine::initialize(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    menu_bg_texture = IMG_LoadTexture(renderer, "assets/images/menu.jpg");
    if (!menu_bg_texture) {
        std::cout << "Bilgi: 'assets/images/menu.jpg' bulunamadi, duz renk arka plan kullanilacak." << std::endl;
    }

    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) return false;

    std::filesystem::create_directories("saves");
    
    scan_images_directory();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    current_state = GameState::MAIN_MENU;
    is_running = true;
    return true;
}

void GameEngine::run() {
    SDL_Event event;

    while (is_running) { 
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                if (current_state == GameState::PLAYING) save_game();
                is_running = false;
            }

            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse) continue;

            if (current_state == GameState::PLAYING) {
                int prev_locked = locked_pieces_count;
                InputHandler::handle_mouse_event(event, pieces, selected_piece, locked_pieces_count, is_victory);
                if (locked_pieces_count > prev_locked) {
                    save_game();
                }
            }
        }

        if (current_state == GameState::PLAYING && !is_victory) {
            elapsed_seconds = saved_elapsed_seconds + (SDL_GetTicks() - start_ticks) / 1000.0f;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // ==================== DURUM 1: ANA MENÜ VE GÖRSEL SEÇİM EKRANI ====================
        if (current_state == GameState::MAIN_MENU) {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(740, 480), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.85f);

            ImGui::Begin("ANA MENU VE GORSEL GALERISI", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "1. Oyuncu Profili:");
            ImGui::InputText("Kullanici Adi", username_buffer, IM_ARRAYSIZE(username_buffer));

            if (has_save_file()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "[!] '%s' icin kayitli bir oyun bulundu.", username_buffer);
                if (ImGui::Button("Kaldigin Yerden Devam Et ", ImVec2(-1, 35))) {
                    load_game();
                }
            } else {
                ImGui::TextDisabled("Bu kullanici icin kayitli oyun bulunmuyor.");
            }

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Columns(2, "GalleryColumns", true);
            ImGui::SetColumnWidth(0, 420);

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "2. Yapboz Gorseli ve Zorluk Secimi:");
            if (ImGui::Button("Klasoru Yenile ")) {
                scan_images_directory();
            }

            ImGui::BeginChild("ImageList", ImVec2(0, 160), true);
            for (int i = 0; i < static_cast<int>(available_images.size()); ++i) {
                std::string filename = std::filesystem::path(available_images[i]).filename().string();
                bool is_selected = (selected_image_index == i);
                if (ImGui::Selectable(filename.c_str(), is_selected)) {
                    if (selected_image_index != i) {
                        selected_image_index = i;
                        update_menu_preview(i); 
                    }
                }
            }
            ImGui::EndChild();

            ImGui::SliderInt("Zorluk (NxN)", &grid_size, 2, 8);

            if (ImGui::Button("Secili Gorselle Yeni Oyun Baslat", ImVec2(-1, 40))) {
                start_new_game();
                save_game();
            }

            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Secilen Gorsel Onizlemesi:");
            if (menu_preview_texture != nullptr) {
                ImGui::Image((ImTextureID)(intptr_t)menu_preview_texture, ImVec2(240, 240));
            } else {
                ImGui::Dummy(ImVec2(240, 240));
                ImGui::TextDisabled("Onizleme Yok");
            }

            ImGui::Columns(1);
            ImGui::Separator();

            if (ImGui::Button("Oyundan Cik", ImVec2(150, 30))) {
                is_running = false;
            }
            if (!status_message.empty()) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", status_message.c_str());
            }

            ImGui::End();
        }
        // ==================== DURUM 2: OYUN EKRANI ====================
        else if (current_state == GameState::PLAYING) {
            ImGui::Begin("Tesella Kontrol Paneli");
            ImGui::Text("Aktif Oyuncu: %s", username_buffer);
            ImGui::Text("Gorsel: %s", std::filesystem::path(current_image_path).filename().string().c_str());
            ImGui::Separator();

            if (is_victory) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "TEBRIKLER! YAPBOZ TAMAMLANDI!");
                ImGui::Text("Tamamlama Suresi: %.1f saniye", elapsed_seconds);
            } else {
                ImGui::Text("Kilitlenen Parca: %d / %d", locked_pieces_count, static_cast<int>(pieces.size()));
                ImGui::Text("Gecen Sure: %.1f sn", elapsed_seconds);
            }

            ImGui::Separator();
            ImGui::Checkbox("Ipucu Resmini Goster", &show_hint);

            if (ImGui::Button("Oyunu Kaydet")) {
                save_game();
            }
            ImGui::SameLine();
            if (ImGui::Button("Kaydet ve Ana Menuye Don")) {
                save_game();
                current_state = GameState::MAIN_MENU;
            }

            if (!status_message.empty()) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", status_message.c_str());
            }
            ImGui::End();
        }

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 25, 25, 30, 255);
        SDL_RenderClear(renderer);

        if (current_state == GameState::MAIN_MENU) {
            if (menu_bg_texture != nullptr) {
                SDL_RenderCopy(renderer, menu_bg_texture, NULL, NULL);
            }
        }


        if (current_state == GameState::PLAYING) {
            SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255); 
            SDL_Rect board_rect = {board_offset_x, board_offset_y, board_w, board_h}; 
            SDL_RenderFillRect(renderer, &board_rect);

            if (show_hint && preview_texture != nullptr) {
                SDL_RenderCopy(renderer, preview_texture, NULL, &board_rect);
            }

            SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255); 
            SDL_RenderDrawRect(renderer, &board_rect);

            for (auto& piece : pieces) {
                if (piece.get_is_locked()) piece.render(renderer);
            }
            for (auto& piece : pieces) {
                if (!piece.get_is_locked()) piece.render(renderer);
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
    if (preview_texture != nullptr) {
        SDL_DestroyTexture(preview_texture);
    }
    if (menu_preview_texture != nullptr) {
        SDL_DestroyTexture(menu_preview_texture);
    }
    if (menu_bg_texture != nullptr) {
        SDL_DestroyTexture(menu_bg_texture);
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}