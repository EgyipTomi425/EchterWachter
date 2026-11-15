# Pure gateway, command and voice decision logic as named modules. It has no
# D++ dependency, so its tests need neither a Discord token nor the cluster.
add_library(echterwachter_core)
set_target_properties(echterwachter_core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_sources(echterwachter_core
        PUBLIC
        FILE_SET CXX_MODULES FILES
            command_registry.cppm
            gateway_health.cppm
            voice_connection.cppm
        PRIVATE
            gateway_health.cpp
            voice_connection.cpp
)

add_library(echterwachter)

# Windows
set_target_properties(echterwachter PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(CURL REQUIRED)

include(FetchContent)

get_property(_old_BSL CACHE BUILD_SHARED_LIBS PROPERTY VALUE)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Force shared build for DPP" FORCE)

FetchContent_Declare(
        DPP
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
            GIT_TAG master
            SYSTEM
)
set(DPP_BUILD_TEST OFF CACHE BOOL "Do not build D++ examples" FORCE)
set(DPP_INSTALL OFF CACHE BOOL "Do not install the embedded D++ dependency" FORCE)
FetchContent_MakeAvailable(DPP)
include("${PROJECT_SOURCE_DIR}/cmake/dpp-json-modules.cmake")

set(BUILD_SHARED_LIBS ${_old_BSL} CACHE BOOL "Restore previous BUILD_SHARED_LIBS" FORCE)

target_sources(echterwachter
        PUBLIC
        FILE_SET CXX_MODULES FILES
            echterwachter.cppm
        PRIVATE
            echterwachter.cpp
)

target_link_libraries(echterwachter PUBLIC dpp CURL::libcurl PRIVATE echterwachter_core)

target_compile_definitions(echterwachter PUBLIC ECHTERWACHTER)
