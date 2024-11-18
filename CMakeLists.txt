cmake_minimum_required(VERSION 3.28)
project(tetris__)

set(CMAKE_CXX_STANDARD 17)
find_package(SDL2 REQUIRED)
set(SDL2_IMAGE_LIBRARIES "/lib/libSDL2_image.so")
add_executable(tetris__ main.cpp
        garbage.h)

target_link_libraries(tetris__ ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
