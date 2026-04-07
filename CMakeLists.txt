cmake_minimum_required(VERSION 3.20)
project(Chip8Emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Homebrew prefix for SDL2 on Apple Silicon / Intel Macs
# SDL2 installed via: brew install sdl2
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

find_package(SDL2 REQUIRED)

add_executable(chip8
    src/main.cpp
    src/Display.cpp
    src/Input.cpp
    src/Chip8.cpp
)

target_include_directories(chip8 PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES})
