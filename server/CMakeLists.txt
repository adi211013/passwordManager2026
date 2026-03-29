file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")

add_executable(server_app ${SERVER_SOURCES}
        src/routes/AuthRoutes.cpp
        src/routes/AuthRoutes.h
        src/routes/VaultRoutes.cpp
        src/routes/VaultRoutes.h
        src/routes/LogRoutes.cpp
        src/routes/LogRoutes.h
        src/config/AppConfig.h
        )

target_include_directories(server_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(server_app PRIVATE ${PQXX_INCLUDE_DIRS} ${SODIUM_INCLUDE_DIRS})

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