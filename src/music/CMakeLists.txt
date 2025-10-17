add_library(music)

target_sources(music
        PUBLIC
        FILE_SET CXX_MODULES FILES
            music.ixx
)

if(SPOTIFY)
    add_subdirectory(spotify)
    target_link_libraries(music PUBLIC spotify)
endif()

if(YOUTUBE)
    add_subdirectory(youtube)
    target_link_libraries(music PUBLIC youtube)
endif()
