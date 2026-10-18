#include "../include/game_engine.h"
#include "../include/puzzle_generator.h"
#include "../include/input_handler.h"
#include <iostream>
#include <fstream>
#include <filesystem>
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

void GameEngine::start_new_game() {
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
    status_message = "Yeni oyun baslatildi.";
}

bool GameEngine::save_game() {
    std::filesystem::create_directories("saves");
    std::ofstream file(get_save_path());
    if (!file.is_open()) return false;

    // 1. Genel oyun bilgilerini yaz
    file << current_image_path << "\n";
    file << grid_size << " " << locked_pieces_count << " " << is_victory << " " << elapsed_seconds << "\n";
    file << pieces.size() << "\n";

    // 2. Her parçanın ID, konum, kilit durumu ve 4 kenar bilgisini yaz (Çizim sırasıyla)
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

    status_message = "Oyun basariyla kaydedildi: " + get_save_path();
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

    // Kayıtlı kenar kalıplarını kullanarak dokuları yeniden üret
    if (!PuzzleGenerator::generate_pieces(
        renderer, current_image_path,
        grid_size, grid_size,
        board_offset_x, board_offset_y,
        board_w, board_h,
        preview_texture, pieces,
        &custom_edges
    )) {
        return false;
    }

    // Üretilen parçaları kayıttaki Z-Order (çizim sırası) ve koordinatlarına göre yerleştir
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

    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) return false;

    std::filesystem::create_directories("saves");

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
                if (current_state == GameState::PLAYING) save_game(); // Çıkarken otomatik kaydet
                is_running = false;
            }

            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse) continue;

            // Sadece PLAYING durumundayken fareyle yapboz parçalarını hareket ettir
            if (current_state == GameState::PLAYING) {
                int prev_locked = locked_pieces_count;
                InputHandler::handle_mouse_event(event, pieces, selected_piece, locked_pieces_count, is_victory);
                // Her parça kilitlendiğinde otomatik kaydet (Auto-Save)
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

        // ==================== DURUM 1: ANA MENÜ ====================
        if (current_state == GameState::MAIN_MENU) {
            ImGui::SetNextWindowPos(ImVec2(390, 200), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(500, 320), ImGuiCond_Always);
            ImGui::Begin("TESELLA - ANA MENU", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Hos Geldiniz! Lutfen kullanici adinizi girin:");
            ImGui::InputText("Kullanici Adi", username_buffer, IM_ARRAYSIZE(username_buffer));
            ImGui::Separator();

            bool save_exists = has_save_file();
            if (save_exists) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "[!] '%s' icin kayitli bir oyun bulundu.", username_buffer);
                if (ImGui::Button("Kaldigin Yerden Devam Et (Load Game)", ImVec2(-1, 40))) {
                    load_game();
                }
            } else {
                ImGui::TextDisabled("Bu kullanici icin henuz kayitli bir oyun yok.");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Yeni Oyun Ayarlari:");
            ImGui::SliderInt("Zorluk (NxN Izgara)", &grid_size, 2, 8);

            if (ImGui::Button("Yeni Oyun Baslat", ImVec2(-1, 40))) {
                start_new_game();
                save_game(); // Yeni oyunu hemen kaydet
            }

            ImGui::Spacing();
            if (ImGui::Button("Oyundan Cik", ImVec2(-1, 30))) {
                is_running = false;
            }

            if (!status_message.empty()) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", status_message.c_str());
            }

            ImGui::End();
        }
        // ==================== DURUM 2: OYUN EKRANI ====================
        else if (current_state == GameState::PLAYING) {
            ImGui::Begin("Tesella Kontrol Paneli");
            ImGui::Text("Aktif Oyuncu: %s", username_buffer);
            ImGui::Separator();

            if (is_victory) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "TEBRIKLER! YAPBOZ TAMAMLANDI!");
                ImGui::Text("Tamamlama Suresi: %.1f saniye", elapsed_seconds);
            } else {
                ImGui::Text("Kilitlenen Parca: %d / %d", locked_pieces_count, static_cast<int>(pieces.size()));
                ImGui::Text("Gecen Sure: %.1f sn", elapsed_seconds);
            }

            ImGui::Separator();
            ImGui::Checkbox("Ipucu Resmini Goster (Hayalet Mod)", &show_hint);

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

        // Sadece PLAYING modundayken oyun tahtasını ve parçaları çiz
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
    
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}