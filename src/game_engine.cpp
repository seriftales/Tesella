#include "../include/game_engine.h"
#include <iostream>
#include <SDL2/SDL_image.h>



GameEngine::GameEngine() {
    window = nullptr;
    renderer = nullptr;
    test_texture = nullptr;
    is_running = false;
}

GameEngine::~GameEngine() {}

bool GameEngine::initialize(const char* title, int width, int height) {
    // SDL ve Pencere başlatma işlemleri (önceki kodlar aynı)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL başlatılamadı! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    // Görüntü motorunu PNG desteğiyle başlat
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image başlatılamadı! Hata: " << IMG_GetError() << std::endl;
        return false;
    }

    // Test görselini doğrudan GPU belleğine (VRAM) yükle
    test_texture = IMG_LoadTexture(renderer, "assets/images/debug_grid.png");
    if (!test_texture) {
        std::cerr << "Görsel yüklenemedi! Dosya yolunu kontrol edin. Hata: " << IMG_GetError() << std::endl;
        return false;
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
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Tesella Kontrol Paneli");
        ImGui::Text("Görsel GPU'ya yüklendi ve çiziliyor.");
        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Görselin ekranda çizileceği koordinat ve boyutları belirle
        SDL_Rect dest_rect = {50, 50, 600, 600};
        SDL_RenderCopy(renderer, test_texture, NULL, &dest_rect);

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
}

void GameEngine::clean() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Doku belleğini ve görüntü kütüphanesini temizle
    SDL_DestroyTexture(test_texture);
    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Motor temizlendi ve kapatıldı." << std::endl;
}