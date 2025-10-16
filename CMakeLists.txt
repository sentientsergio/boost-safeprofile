cmake_minimum_required(VERSION 3.20)

project(boost-safeprofile
    VERSION 0.0.1
    DESCRIPTION "C++ Safety Profile conformance analysis tool"
    LANGUAGES CXX
)

# Project-wide settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type defaults
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# CMake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Import build helpers
include(CompilerWarnings)
include(Sanitizers)
include(Dependencies)

# Options
option(BOOST_SAFEPROFILE_BUILD_TESTS "Build test suite" ON)
option(BOOST_SAFEPROFILE_ENABLE_SANITIZERS "Enable sanitizers in debug builds" ON)

# Core executable
add_executable(boost-safeprofile
    src/main.cpp
    src/cli/arguments.cpp
)

target_include_directories(boost-safeprofile PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Apply compiler warnings
set_project_warnings(boost-safeprofile)

# Apply sanitizers in debug builds
if(BOOST_SAFEPROFILE_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_sanitizers(boost-safeprofile)
endif()

# Link dependencies
target_link_libraries(boost-safeprofile PRIVATE
    Boost::program_options
    Boost::filesystem
    Boost::json
)

# Installation
install(TARGETS boost-safeprofile
    RUNTIME DESTINATION bin
)

# Tests
if(BOOST_SAFEPROFILE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
