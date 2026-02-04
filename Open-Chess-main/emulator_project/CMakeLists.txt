cmake_minimum_required(VERSION 3.16)
project(OpenChessEmulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define project paths
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(FIRMWARE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(emulator)
add_subdirectory(firmware_host)
