cmake_minimum_required(VERSION 3.22)
project(passwordManager2026 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Opcje budowania
option(BUILD_SERVER "Buduj serwer" ON)
option(BUILD_CLIENT "Buduj klienta Qt6" OFF)

# Zależności serwera — ładuj tylko gdy potrzebne
if(BUILD_SERVER)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED libpqxx)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    find_package(OpenSSL REQUIRED)

    include(FetchContent)

    FetchContent_Declare(
            Crow
            GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
            GIT_TAG master
    )
    FetchContent_Declare(
            jwt_cpp
            GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
            GIT_TAG master
    )
    FetchContent_MakeAvailable(Crow jwt_cpp)

    message(STATUS "Buduje serwer...")
    add_subdirectory(shared)
    add_subdirectory(server)
endif()

if(BUILD_CLIENT)
    message(STATUS "Buduje klienta Qt6...")
    add_subdirectory(client)
endif()