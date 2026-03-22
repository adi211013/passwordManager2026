file(GLOB_RECURSE SHARED_SOURCES "*.cpp")

if(SHARED_SOURCES)
    add_library(shared_lib STATIC ${SHARED_SOURCES})
    target_include_directories(shared_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
else()
    add_library(shared_lib INTERFACE)
    target_include_directories(shared_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()