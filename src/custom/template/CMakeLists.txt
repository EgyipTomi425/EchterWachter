add_library(template)

target_sources(template
        PUBLIC
        FILE_SET CXX_MODULES FILES
            template.cppm
            test.cppm
        PRIVATE
            test.cpp
)

# If you want use the discord bot as interface or any from UTIL
# target_link_libraries(template PUBLIC echterwachter)

target_compile_definitions(template PUBLIC TEMPLATE)