add_library(spotify)

target_sources(spotify
        PUBLIC
        FILE_SET CXX_MODULES FILES
            spotify.ixx
            test.ixx
        PRIVATE
            test.cppm
)

target_compile_definitions(spotify PUBLIC SPOTIFY)
