cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(EMULATOR_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ChessBoardWidget.cpp
    src/TcpServer.cpp
)

add_executable(ChessEmulator
    ${EMULATOR_SOURCES}
)

target_link_libraries(ChessEmulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

target_include_directories(ChessEmulator PRIVATE src)
