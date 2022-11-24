cmake_minimum_required(VERSION 3.22)
project(createCFG)

add_compile_options(-std=c++17)

include_directories(include)

file(GLOB SOURCES "*.cpp")

add_executable(createCFG ${SOURCES})
target_link_libraries(createCFG PRIVATE jsoncpp)
