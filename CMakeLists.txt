cmake_minimum_required(VERSION 4.0)

project(EchterWachter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        DPP
        GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
        GIT_TAG master
)
FetchContent_MakeAvailable(DPP)

include("${CMAKE_SOURCE_DIR}/config.cmake")

add_executable(EchterWachter)

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

target_link_libraries(EchterWachter PRIVATE dpp)