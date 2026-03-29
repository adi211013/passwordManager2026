file(GLOB_RECURSE SERVER_SOURCES src/*.cpp src/*.h)

add_executable(server_app ${SERVER_SOURCES})

target_include_directories(server_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${PQXX_INCLUDE_DIRS}
        ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(server_app PRIVATE
        shared_lib
        Crow::Crow
        jwt-cpp::jwt-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        ${PQXX_LIBRARIES}
        ${SODIUM_LIBRARIES}
        pthread
)