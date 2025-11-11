cmake_minimum_required(VERSION 3.10)
project(SmartDataProcessing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(SmartDataProcessing ${SOURCES})

# Link pthread for multithreading
find_package(Threads REQUIRED)
target_link_libraries(SmartDataProcessing Threads::Threads)

# Output directory
set_target_properties(SmartDataProcessing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Print configuration
message(STATUS "=== Build Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Build Directory: ${CMAKE_BINARY_DIR}")
