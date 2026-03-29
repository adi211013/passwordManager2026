cmake_minimum_required(VERSION 3.16)

project(PasswordManagerClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CLIENT_SOURCES
        src/main.cpp
        src/MainWindow/MainWindow.cpp
        src/MainWindow/MainWindow.h
        src/LoginWidget/LoginWidget.h
        src/LoginWidget/LoginWidget.cpp
        src/VaultWidget/VaultWidget.h
)

add_executable(client_app WIN32 MACOSX_BUNDLE ${CLIENT_SOURCES})

target_link_libraries(client_app PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)