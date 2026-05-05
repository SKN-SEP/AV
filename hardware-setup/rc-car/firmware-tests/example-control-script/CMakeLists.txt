cmake_minimum_required(VERSION 3.10.0)
project(example-control-script VERSION 0.1.0 LANGUAGES C CXX)

add_executable(example-control-script main.cpp serialib.cpp serialib.h)