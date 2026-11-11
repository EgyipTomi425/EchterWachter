add_executable(gateway_health_tests gateway_health_tests.cpp)
target_include_directories(gateway_health_tests PRIVATE ../src/dpp)
add_test(NAME gateway_health COMMAND gateway_health_tests)
set_tests_properties(gateway_health PROPERTIES TIMEOUT 5)

add_executable(command_registry_tests command_registry_tests.cpp)
target_include_directories(command_registry_tests PRIVATE ../src/dpp)
add_test(NAME command_registry COMMAND command_registry_tests)
set_tests_properties(command_registry PROPERTIES TIMEOUT 5)

add_test(NAME module_initialization COMMAND EchterWachter --check-modules)
set_tests_properties(module_initialization PROPERTIES
    ENVIRONMENT "DISCORD_TOKEN=offline-module-check"
    TIMEOUT 10)
