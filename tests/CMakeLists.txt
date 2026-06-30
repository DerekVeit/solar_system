add_executable(solar_system_tests
    test_horizons.cpp
    test_kepler.cpp
    test_json_loader.cpp
    test_types.cpp
)

target_link_libraries(solar_system_tests
    PRIVATE
        solar_core
        Catch2::Catch2WithMain
)

add_custom_command(TARGET solar_system_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tests/data
        ${CMAKE_BINARY_DIR}/tests/data
    COMMENT "Copy assets and Horizons reference data for unit tests"
)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)
catch_discover_tests(solar_system_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)