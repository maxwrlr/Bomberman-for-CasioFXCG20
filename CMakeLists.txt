cmake_minimum_required(VERSION 3.8)
project(Bomberman)
include_directories("C:/Users/Max/Desktop/Prizm/PrizmSDK-0.3/include/")

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES src/main.c src/world.c src/bomberman.c src/bombs.c src/draw.c src/astar.c src/menu.c src/menu.c src/key.c)
add_executable(Bomberman ${SOURCE_FILES})