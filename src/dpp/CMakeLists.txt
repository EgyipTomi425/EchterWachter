add_library(echterwachter)

include(FetchContent)

find_package(DPP)

if(NOT DPP_FOUND)
    message(STATUS "DPP not found, downloading via FetchContent.")
    FetchContent_Declare(
            DPP
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
            GIT_TAG master
    )
    FetchContent_MakeAvailable(DPP)
else()
    message(STATUS "DPP found, skipping download.")
endif()

target_sources(echterwachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            echterwachter.cppm
        PRIVATE
            echterwachter.cpp
)

target_link_libraries(echterwachter PUBLIC dpp)

target_compile_definitions(echterwachter PUBLIC ECHTERWACHTER)
