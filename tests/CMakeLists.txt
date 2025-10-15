# Test suite

find_package(Boost 1.82 REQUIRED COMPONENTS unit_test_framework)

# Unit tests
add_executable(unit_tests
    unit/test_main.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(unit_tests PRIVATE
    Boost::unit_test_framework
)

add_test(NAME unit_tests COMMAND unit_tests)

# Integration tests will be added as shell scripts in later phases
