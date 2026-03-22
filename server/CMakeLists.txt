file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")

add_executable(server_app ${SERVER_SOURCES})

target_include_directories(server_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(server_app PRIVATE ${PQXX_INCLUDE_DIRS} ${SODIUM_INCLUDE_DIRS})

target_link_libraries(server_app PRIVATE
        shared_lib                 # Nasza biblioteka ze struktury shared/
        Crow::Crow                 # Framework webowy
        jwt-cpp::jwt-cpp           # Tokeny JWT
        OpenSSL::SSL               # Wymagane
        OpenSSL::Crypto            # Wymagane
        ${PQXX_LIBRARIES}          # Baza danych z pkg_config
        ${SODIUM_LIBRARIES}        # Libsodium z pkg_config
        pthread                    # Wątki dla serwera
)