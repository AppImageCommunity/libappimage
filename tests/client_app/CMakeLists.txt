project(client_app)
cmake_minimum_required(VERSION 3.0)

add_executable(client_app main.c)
target_link_libraries(client_app libappimage)

add_executable(client_app_static_linked main.c)
target_link_libraries(client_app_static_linked libappimage_static)
