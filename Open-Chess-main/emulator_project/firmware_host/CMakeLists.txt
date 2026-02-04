cmake_minimum_required(VERSION 3.16)

# Firmware sources are in the parent folder, but we need to compile them here
# with our mocks.

set(FIRMWARE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../Open-Chess-main)

# Source files from the original firmware
set(FIRMWARE_SOURCES
    ${FIRMWARE_ROOT}/chess_engine.cpp
    ${FIRMWARE_ROOT}/chess_moves.cpp
    ${FIRMWARE_ROOT}/chess_bot.cpp
    ${FIRMWARE_ROOT}/sensor_test.cpp
    # Note: using local RP2040 wifi manager
    # ${FIRMWARE_ROOT}/wifi_manager_rp2040.cpp
    # board_driver handled by mock
)

# New host files
set(HOST_SOURCES
    src/main_pc.cpp
    src/board_driver_mock.cpp
    # src/OpenChess.ino is included via #include in main_pc.cpp, so not added here directly
)

add_executable(FirmwareHost
    ${HOST_SOURCES}
    ${FIRMWARE_SOURCES}
)

# Include paths:
# 1. Our mock implementations (Arduino.h, etc)
# 2. Original source include path
target_include_directories(FirmwareHost PRIVATE
    include
    ${FIRMWARE_ROOT}
)

# Link threads
find_package(Threads REQUIRED)
target_link_libraries(FirmwareHost PRIVATE Threads::Threads)
