project(client_app)
cmake_minimum_required(VERSION 3.8)

find_package(libappimage REQUIRED)
message("LINK_DEPENDENCIES ${LINK_DEPENDENCIES}")

add_executable(client_app main.c)
target_link_libraries(client_app libappimage)
