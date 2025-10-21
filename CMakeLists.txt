cmake_minimum_required(VERSION 4.0)

project(EchterWachter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 26)
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

if(GGE_SDK_DC)
    target_compile_definitions(EchterWachter PRIVATE GGE_SDK_DC)
    add_subdirectory(src/gge-sdk/discord)
    target_link_libraries(EchterWachter PRIVATE gge_sdk_dc)
endif()

target_sources(EchterWachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            src/plugins.cppm
        PRIVATE
            src/main.cpp
)