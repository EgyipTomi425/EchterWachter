add_library(echterwachter)

include(FetchContent)
FetchContent_Declare(
        DPP
        GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
        GIT_TAG master
)
FetchContent_MakeAvailable(DPP)

target_sources(echterwachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            echterwachter.ixx
        PRIVATE
            echterwachter.cppm
)

target_link_libraries(echterwachter PUBLIC dpp)

target_compile_definitions(echterwachter PUBLIC ECHTERWACHTER)
