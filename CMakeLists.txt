cmake_minimum_required(VERSION 4.0)

project(EchterWachter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_SOURCE_DIR}/config.cmake")

add_executable(EchterWachter)

# Discord lib interface
target_compile_definitions(EchterWachter PRIVATE ECHTERWACHTER)
add_subdirectory(src/dpp)
target_link_libraries(EchterWachter PRIVATE echterwachter)
#######################

# Not discord related useful moduls
if(UTIL)
    target_compile_definitions(EchterWachter PRIVATE UTIL)
    add_subdirectory(src/util)
    target_link_libraries(EchterWachter PRIVATE util)
endif()
####################################

# Discord related commands and modules (They can use UTIL)
if(MUSIC)
    target_compile_definitions(EchterWachter PRIVATE MUSIC)
    add_subdirectory(src/music)
    target_link_libraries(EchterWachter PRIVATE music)
endif()
######################################

# Custom plugins
set(CUSTOM OFF)
if(EXISTS "${CMAKE_SOURCE_DIR}/src/custom")
    file(GLOB CUSTOM_CONTENT "${CMAKE_SOURCE_DIR}/src/custom/*")
    if(CUSTOM_CONTENT)
        message(STATUS "Custom content found, enabling CUSTOM.")
        set(CUSTOM ON CACHE BOOL "Enable custom libraries")
        target_compile_definitions(EchterWachter PRIVATE CUSTOM)
        add_subdirectory(src/custom)
        target_link_libraries(EchterWachter PRIVATE custom)
    else()
        message(STATUS "Custom folder exists but is empty, skipping custom libraries.")
    endif()
else()
    message(STATUS "Custom folder does not exist, skipping custom libraries.")
endif()
################

target_sources(EchterWachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            src/plugins.cppm
        PRIVATE
            src/main.cpp
)