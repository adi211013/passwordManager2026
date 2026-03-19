cmake_minimum_required(VERSION 4.1)
project(passwordManager2026)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(SODIUM REQUIRED libsodium) #
include(FetchContent)
FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG master
)
FetchContent_MakeAvailable(Crow)

add_executable(passwordManager2026 src/main.cpp
        src/database/DbManager.cpp
        src/database/DbManager.h
        src/crypto/Crypto.cpp
        src/crypto/Crypto.h)

target_include_directories(passwordManager2026 PRIVATE ${PQXX_INCLUDE_DIRS} ${SODIUM_INCLUDE_DIRS})
target_link_libraries(passwordManager2026 PRIVATE Crow::Crow ${PQXX_LIBRARIES} ${SODIUM_LIBRARIES})