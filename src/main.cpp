#include <iostream>
#include "game_engine.h"

int main(int argc, char* argv[]) {

    GameEngine engine;

    if (!engine.initialize("Tesella", 1600, 900)) {
        std::cerr << "Kritik Hata: Motor başlatılamadı. Program sonlandırılıyor." << std::endl;
        return -1; 
    }
    engine.run();

    engine.clean();

    return 0; 
}