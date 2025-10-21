add_library(qt)

find_package(Qt6 COMPONENTS Core REQUIRED)

target_sources(qt
        PUBLIC
        FILE_SET CXX_MODULES FILES
            qt.cppm
)

target_link_libraries(qt PUBLIC Qt6::Core)
target_compile_definitions(qt PUBLIC QT)