cmake_minimum_required(VERSION 4.0)

project(EchterWachter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_SOURCE_DIR}/config.cmake")

add_executable(EchterWachter)

target_compile_definitions(EchterWachter PRIVATE ECHTERWACHTER)
add_subdirectory(src/dpp)
target_link_libraries(EchterWachter PRIVATE echterwachter)

if(MUSIC)
    target_compile_definitions(EchterWachter PRIVATE MUSIC)
    add_subdirectory(src/music)
    target_link_libraries(EchterWachter PRIVATE music)
endif()

target_sources(EchterWachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            src/plugins.ixx
        PRIVATE
            src/main.cpp
)