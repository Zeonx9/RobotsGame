cmake_minimum_required(VERSION 3.20)
project(RobotsGame)

set(CMAKE_CXX_STANDARD 11)

add_executable(RobotsGame test_dir/main.cpp app_client/game.h app_client/game.c)
add_executable(Server app_server/main.c app_server/server.c app_server/server.h app_server/requests.c app_server/requests.h sqlite/sqlite3.c SQL/database.c SQL/database.h SQL/data_scheme.c SQL/data_scheme.h)
add_executable(Client app_client/client.c app_client/client.h app_client/interface.cpp app_client/interface.h app_client/main.cpp app_client/intfc_classes.cpp app_client/intfc_classes.h app_client/game.c app_client/game.h SQL/data_scheme.c SQL/data_scheme.h)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window audio graphics network REQUIRED)

include_directories(C:/SFML/include)
target_link_libraries(RobotsGame sfml-graphics sfml-audio)
target_link_libraries(Server ws2_32)
target_link_libraries(Client ws2_32 sfml-graphics sfml-audio)