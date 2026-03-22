cmake_minimum_required(VERSION 3.22)
project(passwordManager2026 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
FetchContent_MakeAvailable(Crow)

FetchContent_Declare(
        jwt_cpp
        GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(jwt_cpp)

add_subdirectory(shared)
add_subdirectory(server)