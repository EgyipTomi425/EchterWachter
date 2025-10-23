add_library(custom)

include("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")

target_sources(custom
        PUBLIC
        FILE_SET CXX_MODULES FILES
            custom.cppm
)

# Your custom libraries are here
# You can use config.cmake variables to make it optional
if(TEMPLATE)
    add_subdirectory(template)
    target_link_libraries(custom PUBLIC template)
endif()