#include "../include/game_engine.h"
#include <iostream>
#include <SDL2/SDL_image.h>

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

    test_texture = IMG_LoadTexture(renderer, "assets/images/debug_grid.png");
    if (!test_texture) {
        std::cerr << "Görsel yüklenemedi! Dosya yolunu kontrol edin. Hata: " << IMG_GetError() << std::endl;
        return false;
    }
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int id = row * 4 + col;
            int src_x = col * 150; 
            int src_y = row * 150; 
            
            int dest_x = 800 + (col * 30); 
            int dest_y = 100 + (row * 30); 
            
            pieces.emplace_back(id, src_x, src_y, 150, 150, dest_x, dest_y);
        }
    
    }

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
            piece.render(renderer, test_texture);
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        
    } 
}

void GameEngine::clean() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(test_texture);
    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Motor temizlendi ve kapatıldı." << std::endl;
}