cmake_minimum_required(VERSION 3.16)

project(PasswordManagerClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_library(SODIUM_LIB sodium HINTS /opt/homebrew/lib)
find_path(SODIUM_INCLUDE sodium.h HINTS /opt/homebrew/include)

file(GLOB_RECURSE CLIENT_SOURCES
        src/*.cpp
        src/*.h
)

add_executable(client_app ${CLIENT_SOURCES})

target_include_directories(client_app PRIVATE
        src
        ${SODIUM_INCLUDE}
)

target_link_libraries(client_app PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        ${SODIUM_LIB}
)