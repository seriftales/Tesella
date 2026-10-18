cmake_minimum_required(VERSION 3.10)

project(tesella)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(include)
include_directories(vendor/imgui)

file(GLOB IMGUI_SOURCES 
    "vendor/imgui/imgui.cpp"
    "vendor/imgui/imgui_draw.cpp"
    "vendor/imgui/imgui_tables.cpp"
    "vendor/imgui/imgui_widgets.cpp"
    "vendor/imgui/imgui_impl_sdl2.cpp"
    "vendor/imgui/imgui_impl_sdlrenderer2.cpp"
)

add_executable(tesella src/main.cpp src/game_engine.cpp src/puzzle_piece.cpp src/puzzle_generator.cpp src/input_handler.cpp ${IMGUI_SOURCES})

target_link_libraries(tesella 
    ${SDL2_LIBRARIES} 
    SDL2_image 
    SDL2_ttf 
    SDL2_mixer
)

target_include_directories(tesella PRIVATE ${SDL2_INCLUDE_DIRS})