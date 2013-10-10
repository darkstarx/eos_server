cmake_minimum_required(VERSION 2.8)
project(eos_server)

find_package(Boost 1.49 COMPONENTS thread REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils ${CMAKE_SOURCE_DIR}/server)

file(GLOB utils_SOURCES ${CMAKE_SOURCE_DIR}/utils/*.hpp ${CMAKE_SOURCE_DIR}/utils/*.cpp)
file(GLOB server_SOURCES ${CMAKE_SOURCE_DIR}/server/*.hpp ${CMAKE_SOURCE_DIR}/server/*.cpp)

add_executable(${PROJECT_NAME} ${utils_SOURCES} ${server_SOURCES} main.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ev)
