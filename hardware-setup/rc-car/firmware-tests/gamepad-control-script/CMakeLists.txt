cmake_minimum_required(VERSION 3.10.0)
project(gamepad-control-script VERSION 0.1.0 LANGUAGES C CXX)


find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(gamepad-control-script main.cpp serialib.cpp serialib.h)

target_link_libraries(gamepad-control-script ${SDL2_LIBRARIES})