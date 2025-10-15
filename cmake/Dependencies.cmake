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

# TODO: Add LLVM/Clang dependencies in later phases
# find_package(LLVM 15 REQUIRED CONFIG)
# find_package(Clang REQUIRED CONFIG)
