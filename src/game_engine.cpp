#include "../include/game_engine.h"
#include "../include/puzzle_generator.h"
#include "../include/input_handler.h"
#include <iostream>
#include <SDL2/SDL_image.h>

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

GameEngine::~GameEngine() {}

bool GameEngine::initialize(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;

    board_offset_x = 150; 
    board_offset_y = 100; 

    // ÜRETİM İŞİNİ PUZZLE GENERATOR SINIFINA DEVRETTİK!
    if (!PuzzleGenerator::generate_pieces(renderer, "assets/images/debug_grid.png", 4, 4, board_offset_x, board_offset_y, board_w, board_h, pieces)) {
        return false;
    }

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

            // FARE KONTROLÜNÜ INPUT HANDLER SINIFINA DEVRETTİK!
            InputHandler::handle_mouse_event(event, pieces, selected_piece, locked_pieces_count, is_victory);
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

        // YENİ ÖZELLİK: Oyun içinden tek tuşla yeniden karıştırma (Reset) butonu!
        if (ImGui::Button("Yeniden Karistir (Reset)")) {
            for (auto& piece : pieces) piece.free_texture();
            selected_piece = nullptr;
            locked_pieces_count = 0;
            is_victory = false;
            PuzzleGenerator::generate_pieces(renderer, "assets/images/debug_grid.png", 4, 4, board_offset_x, board_offset_y, board_w, board_h, pieces);
        }
        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255); 
        SDL_Rect board_rect = {board_offset_x, board_offset_y, board_w, board_h}; 
        SDL_RenderFillRect(renderer, &board_rect);
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255); 
        SDL_RenderDrawRect(renderer, &board_rect);

        for (auto& piece : pieces) {
            if (piece.get_is_locked()) piece.render(renderer);
        }
        for (auto& piece : pieces) {
            if (!piece.get_is_locked()) piece.render(renderer);
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
}