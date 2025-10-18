# Test suite

find_package(Boost 1.82 REQUIRED COMPONENTS unit_test_framework)

# Unit tests
add_executable(unit_tests
    unit/test_main.cpp
    unit/test_cli.cpp
    unit/test_intake.cpp
    unit/test_ast_detector.cpp
    # Source files to test
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/cli/arguments.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/intake/repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/analysis/ast_detector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/profile/loader.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    SYSTEM ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(unit_tests PRIVATE
    Boost::unit_test_framework
    Boost::program_options
    Boost::filesystem
    Boost::json
    clangTooling
    clangFrontend
    clangParse
    clangSerialization
    clangSema
    clangEdit
    clangAnalysis
    clangDriver
    clangASTMatchers
    clangAST
    clangLex
    clangBasic
)

add_test(NAME unit_tests COMMAND unit_tests)

# Integration tests will be added as shell scripts in later phases
