add_library(spotify)

target_sources(spotify
        PUBLIC
        FILE_SET CXX_MODULES FILES
            spotify.cppm
            test.cppm
        PRIVATE
            test.cpp
)

target_link_libraries(spotify PUBLIC echterwachter)

target_compile_definitions(spotify PUBLIC SPOTIFY)