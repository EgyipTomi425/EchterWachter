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
if(CUSTOM)
    set(CUSTOM OFF)
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/custom")
        if(EXISTS "${CMAKE_SOURCE_DIR}/src/custom/TemplateCMakeLists.cmake" AND
                NOT EXISTS "${CMAKE_SOURCE_DIR}/src/custom/CMakeLists.txt")

            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                    "${CMAKE_SOURCE_DIR}/src/custom/TemplateCMakeLists.cmake"
                    "${CMAKE_SOURCE_DIR}/src/custom/CMakeLists.txt")

            message(STATUS "TemplateCMakeLists.cmake copied to CMakeLists.txt")
        endif()

        if(EXISTS "${CMAKE_SOURCE_DIR}/src/custom/Templateconfig.cmake" AND
                NOT EXISTS "${CMAKE_SOURCE_DIR}/src/custom/config.cmake")

            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                    "${CMAKE_SOURCE_DIR}/src/custom/Templateconfig.cmake"
                    "${CMAKE_SOURCE_DIR}/src/custom/config.cmake")

            message(STATUS "Templateconfig.cmake copied to config.cmake")
        endif()

        file(GLOB CUSTOM_CONTENT "${CMAKE_SOURCE_DIR}/src/custom/*")
        if(CUSTOM_CONTENT AND EXISTS "${CMAKE_SOURCE_DIR}/src/custom/CMakeLists.txt")
            message(STATUS "Custom content found, enabling CUSTOM.")
            set(CUSTOM ON CACHE BOOL "Enable custom libraries")
            target_compile_definitions(EchterWachter PRIVATE CUSTOM)
            add_subdirectory(src/custom)
            target_link_libraries(EchterWachter PRIVATE custom)
        else()
            message(STATUS "Custom folder exists but empty or no CMakeLists.txt, skipping custom libraries.")
        endif()
    else()
        message(STATUS "Custom folder does not exist, skipping custom libraries.")
    endif()
endif()
################

target_sources(EchterWachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            src/plugins.cppm
        PRIVATE
            src/main.cpp
)
