#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h> 
#include <SDL2/SDL_image.h>
#include "puzzle_piece.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

enum class GameState {
    MAIN_MENU,
    PLAYING
};

class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    bool initialize(const char* title, int width, int height);
    void run();
    void clean();

private:
    void start_new_game();
    bool save_game();
    bool load_game();
    bool has_save_file() const;
    std::string get_save_path() const;

    void scan_images_directory();
    void update_menu_preview(int index);

    GameState current_state = GameState::MAIN_MENU;
    char username_buffer[64] = "kaan";
    std::string status_message = "";

    std::vector<std::string> available_images;
    int selected_image_index = 0;
    SDL_Texture* menu_preview_texture = nullptr;
    SDL_Texture* menu_bg_texture = nullptr; 


    bool is_running = false;
    int locked_pieces_count = 0; 
    bool is_victory = false;         
    bool show_hint = false;

    int grid_size = 4;
    std::string current_image_path = "";

    Uint32 start_ticks = 0;
    float saved_elapsed_seconds = 0.0f;
    float elapsed_seconds = 0.0f;

    int board_offset_x = 100;
    int board_offset_y = 80;
    int board_w = 600;
    int board_h = 600;

    int win_w = 1280;
    int win_h = 720;

    SDL_AudioDeviceID audio_device = 0;
    Uint8* snap_wav_buffer = nullptr;
    Uint32 snap_wav_length = 0;
    void play_snap_sound();
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* preview_texture = nullptr; 

    Mix_Music* bg_music = nullptr;
    Mix_Chunk* snap_sfx = nullptr;
    Mix_Chunk* victory_sfx = nullptr;

    std::vector<PuzzlePiece> pieces;
    PuzzlePiece* selected_piece = nullptr;

};

#endif