add_library(util)

target_sources(util
        PUBLIC
        FILE_SET CXX_MODULES FILES
            util.cppm
)

if(QT)
    add_subdirectory(qt)
    target_link_libraries(util PUBLIC qt)
endif()
