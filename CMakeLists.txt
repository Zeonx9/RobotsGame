cmake_minimum_required(VERSION 3.22)
project(RobotsGame)

set(CMAKE_CXX_STANDARD 14)

add_executable(RobotsGame test_dir/main.cpp)
add_executable(Server app_server/main.c)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window audio graphics network REQUIRED)

include_directories(C:/SFML/include)
target_link_libraries(RobotsGame sfml-system sfml-graphics sfml-audio sfml-audio)
