add_library(youtube)

target_sources(youtube
        PUBLIC
        FILE_SET CXX_MODULES FILES
            youtube.cppm
            test.cppm
        PRIVATE
            test.cpp
)

target_link_libraries(youtube PUBLIC echterwachter)

target_compile_definitions(youtube PUBLIC YOUTUBE)