add_library(youtube)

target_sources(youtube
        PUBLIC
        FILE_SET CXX_MODULES FILES
            youtube.ixx
            test.ixx
        PRIVATE
            test.cppm
)

target_compile_definitions(youtube PUBLIC YOUTUBE)
