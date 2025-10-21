add_library(music)

target_sources(music
        PUBLIC
        FILE_SET CXX_MODULES FILES
            music.cppm
)

if(YOUTUBE)
    add_subdirectory(youtube)
    target_link_libraries(music PUBLIC youtube)
endif()
