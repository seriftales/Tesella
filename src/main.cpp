#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    // SDL'in video (grafik) alt sistemini başlatmayı dene
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL baslatilamadi Hata: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Test Basarili: SDL2 kütüphanesi baglandi ve Tesella motoru baslatildi!" << std::endl;

    // Belleği temizle ve çık
    SDL_Quit();
    return 0;
}