cmake_minimum_required(VERSION 3.20)
project(RobotsGame)

set(CMAKE_CXX_STANDARD 14)

add_executable(RobotsGame test_dir/main.cpp)
add_executable(Server app_server/main.c app_server/server.c app_server/server.h)
add_executable(Client app_client/client.cpp app_client/client.h app_client/interface.cpp app_client/interface.h)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window audio graphics network REQUIRED)

include_directories(C:/SFML/include)
target_link_libraries(RobotsGame sfml-system sfml-graphics sfml-audio sfml-audio)
target_link_libraries(Server ws2_32)
target_link_libraries(Client ws2_32 sfml-system sfml-graphics sfml-audio sfml-audio)
