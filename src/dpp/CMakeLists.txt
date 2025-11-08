add_library(echterwachter)

# Windows
set_target_properties(echterwachter PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(FetchContent)

get_property(_old_BSL CACHE BUILD_SHARED_LIBS PROPERTY VALUE)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Force shared build for DPP" FORCE)

FetchContent_Declare(
        DPP
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
            GIT_TAG master
)
FetchContent_MakeAvailable(DPP)

set(BUILD_SHARED_LIBS ${_old_BSL} CACHE BOOL "Restore previous BUILD_SHARED_LIBS" FORCE)

target_sources(echterwachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            echterwachter.cppm
        PRIVATE
            echterwachter.cpp
)

target_link_libraries(echterwachter PUBLIC dpp)

target_compile_definitions(echterwachter PUBLIC ECHTERWACHTER)