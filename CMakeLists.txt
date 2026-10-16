#minimum gereksinim sürümü
cmake_minimum_required(VERSION 3.10)

# Projenin adı
project(tesella)

# C++17 standardını kullanmaya zorla
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 ve ek kütüphanelerini sistemde bul
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# header dosyalarımızın yerini derleyiciye göster
include_directories(include)

# Çalıştırılabilir dosyamızı oluştur 
add_executable(tesella src/main.cpp src/game_engine.cpp)
# Link
target_link_libraries(tesella 
    ${SDL2_LIBRARIES} 
    ${SDL2_IMAGE_LIBRARIES} 
    ${SDL2_TTF_LIBRARIES} 
    ${SDL2_MIXER_LIBRARIES}
)

# SDL2 başlık dosyası yollarını projeye dahil et
target_include_directories(tesella PRIVATE ${SDL2_INCLUDE_DIRS})