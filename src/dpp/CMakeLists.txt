add_library(echterwachter)

target_sources(spotify
        PUBLIC
        FILE_SET CXX_MODULES FILES
            echterwachter.ixx
)

target_compile_definitions(echterwachter PUBLIC echterwachter)
