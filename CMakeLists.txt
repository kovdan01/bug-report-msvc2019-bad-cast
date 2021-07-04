cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(bug-report-msvc2019-bad-cast LANGUAGES CXX)

find_package(msgpack REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE msgpackc-cxx)
