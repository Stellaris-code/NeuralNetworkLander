cmake_minimum_required(VERSION 3.1)

project(NeuralNetworkShowcase)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window audio REQUIRED)

add_executable(${PROJECT_NAME} "network.cpp" "network.hpp" "genann.c" "graphics.cpp" "pong.hpp" "pong.cpp"
    "playfield.hpp" "common.hpp" "lander.hpp" "lander.cpp"
    "genetic_operations.hpp" "genetic_operations.cpp")
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window)
