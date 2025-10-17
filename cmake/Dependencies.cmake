# Dependencies.cmake - Find and configure external dependencies

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.82 REQUIRED COMPONENTS
    program_options
    filesystem
    json
)

if(Boost_FOUND)
    message(STATUS "Found Boost ${Boost_VERSION}")
    message(STATUS "  Include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${Boost_LIBRARIES}")
endif()

# LLVM/Clang - for AST-based analysis (Phase 1+)
# On macOS with Homebrew, LLVM is keg-only so we need to set CMAKE_PREFIX_PATH
if(APPLE AND EXISTS /opt/homebrew/opt/llvm)
    set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/llvm" ${CMAKE_PREFIX_PATH})
endif()

find_package(LLVM REQUIRED CONFIG)

if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "  LLVM_DIR: ${LLVM_DIR}")
    message(STATUS "  LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "  LLVM_LIBRARY_DIRS: ${LLVM_LIBRARY_DIRS}")
    message(STATUS "  LLVM_DEFINITIONS: ${LLVM_DEFINITIONS}")

    # Separate LLVM definitions into compile definitions
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

    # Find Clang libraries
    find_package(Clang REQUIRED CONFIG)
    message(STATUS "Found Clang")
endif()
