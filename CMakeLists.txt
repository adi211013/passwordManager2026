cmake_minimum_required(VERSION 4.1)
project(passwordManager2026)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include(FetchContent)
FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG master
)
FetchContent_MakeAvailable(Crow)

add_executable(passwordManager2026 src/main.cpp
        src/database/DbManager.cpp
        src/database/DbManager.h)

target_include_directories(passwordManager2026 PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(passwordManager2026 PRIVATE Crow::Crow ${PQXX_LIBRARIES})