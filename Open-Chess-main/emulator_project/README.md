# OpenChess Emulator Guide

This emulator allows you to run and debug the OpenChess firmware on your PC without physical hardware. It consists of two parts:
1. **Emulator UI (Qt C++)**: Visualizes the board state, sensors, and LEDs. Acts as a TCP Server.
2. **Firmware Host (C++)**: Wraps the original `OpenChess.ino` and C++ logic, running it as a standard desktop process. Connects to the Emulator UI via TCP.

## Prerequisites
- **CMake** (3.16+)
- **Qt6** (Core, Widgets, Network)
- **C++ Compiler** (GCC/Clang/MSVC) supporting C++17

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

## Running the Emulator

1. **Start the GUI Emulator first**:
   ```bash
   ./emulator/ChessEmulator
   # Window should appear, listening on port 2323
   ```

2. **Run the Firmware**:
   In a separate terminal:
   ```bash
   ./firmware_host/FirmwareHost
   # Output should show "Connected to Emulator!"
   ```

## Usage

- **Mouse Drag & Drop**: Moves pieces on the visual board.
  - **Lift**: Triggers sensor "Empty" event (Logic 0)
  - **Drop**: Triggers sensor "Occupied" event (Logic 1)
- **Logs**: The right panel shows all LED commands sent by firmware and Sensor events sent to firmware.
- **Game Logic**: The firmware code runs exactly as it would on Arduino. If you select "Chess Mode" via Serial (simulated), it will start. Note: The current mock sets up the board state automatically.

## Test Scenarios

### Scenario 1: Initial Board Setup Check
1. Start Emulator.
2. Start Firmware.
3. Observe log: Firmware should initialize and clear LEDs.
4. Firmware reads sensors. If virtual board matches expected "Makruk" setup, it proceeds.
   - *Note: Makruk setup is pre-loaded in the emulator widget.*
5. Success Criteria: No red LEDs blinking (error state). Firework animation (simulated log) triggers on start.

### Scenario 2: Piece Lift Detection
1. Drag the white Pawn at (Row 2, Col 0).
2. Log should show: `Sensor [2,0] -> Empty`.
3. Firmware serial out (stdout) should print "Piece lifted...".
4. LED at [2,0] should highlight (if firmware logic highlights selected piece).

### Scenario 3: Valid Move
1. Move white Pawn 1 step forward.
2. Log: `Sensor [2,0] -> Empty`, then `Sensor [3,0] -> Occupied`.
3. Firmware confirms move is valid.
4. LED logic might clear highlights or show confirmation animation.

### Scenario 4: Invalid Move (Rule Test)
1. Move white Pawn backwards or sideways (invalid for Makruk pawn).
2. Firmware logic detects violation.
3. LEDs typically flash RED or indicate error.

### Scenario 5: Connectivity Loss
1. Close Emulator window while Firmware is running.
2. Firmware host should detect disconnection and exit or wait.

## Hardware Mapping

| Virtual | Real Hardware | Protocol |
|---------|---------------|----------|
| GUI Piece | Reed Switch / Hall Sensor | `E <row> <col> <1/0>` (1=Occupied) |
| Widget Color | NeoPixel | `L <row> <col> <r> <g> <b>` |
| `Serial.print` | USB Serial / UART | Mocked to `std::cout` |

## Troubleshooting

- **"Connection Failed"**: Ensure Emulator GUI is running before starting FirmwareHost.
- **Qt not found**: Ensure `qt6-base-dev` or similar is installed and `CMAKE_PREFIX_PATH` is set if needed.
