#include "chess_moves.h"
#include <Arduino.h>

// Expected initial configuration (Makruk)
const char ChessMoves::INITIAL_BOARD[8][8] = {
  {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},  // row 0 (rank 1) White Baseline
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // row 1 (rank 2)
  {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},  // row 2 (rank 3) White Pawns
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // row 3 (rank 4)
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // row 4 (rank 5)
  {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},  // row 5 (rank 6) Black Pawns
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // row 6 (rank 7)
  {'r', 'n', 'b', 'k', 'q', 'b', 'n', 'r'}   // row 7 (rank 8) Black Baseline (K opposite K, Q opposite Q)
};

ChessMoves::ChessMoves(BoardDriver* bd, ChessEngine* ce) : boardDriver(bd), chessEngine(ce) {
    // Initialize board state
    initializeBoard();
}

void ChessMoves::begin() {
    Serial.println("Starting Chess Game Mode...");
    
    // Copy expected configuration into our board state
    initializeBoard();

    // Wait for board setup
    waitForBoardSetup();
    
    Serial.println("Chess game ready to start!");
    boardDriver->fireworkAnimation();

    // Initialize sensor previous state for move detection
    boardDriver->readSensors();
    boardDriver->updateSensorPrev();

    // Initial state: Every square that currently contains a piece must show a white LED
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] != ' ') {
                boardDriver->setSquareLED(r, c, 50, 50, 50); // White
            }
        }
    }
    boardDriver->showLEDs();
}

void ChessMoves::update() {
    boardDriver->readSensors();

    // Look for a piece pickup (Sensor LOW, Prev HIGH)
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
             // Check if board assumes a piece is here AND sensor shows it's gone
             // (We rely on internal board state 'board' to know if a game piece exists, 
             //  and sensor to know if it's physically lifted)
             if (board[row][col] != ' ' && !boardDriver->getSensorState(row, col) && boardDriver->getSensorPrev(row, col)) {
                 
                 // --- PICK EVENT ---
                 char piece = board[row][col];
                 int originRow = row;
                 int originCol = col;
                 
                 Serial.print("Piece lifted from ");
                 Serial.print((char)('a' + col));
                 Serial.println(row + 1);

                 // 1. Origin LED OFF
                 boardDriver->setSquareLED(row, col, 0, 0, 0); 

                 // 2. Generate Moves
                 int moveCount = 0;
                 int moves[28][2];
                 chessEngine->getPossibleMoves(board, row, col, moveCount, moves);

                 // 3 & 4. Highlight Logic (Loop until placed)
                 bool placed = false;
                 while (!placed) {
                     // Re-render highlights every frame (in case of other changes, though mainly static)
                     // or just once? "Update in real-time if board state changes". 
                     // Since we pause here, board state only changes by us or opponent? 
                     // In local mode, only this loop controls state.
                     
                     boardDriver->clearAllLEDs();
                     
                     // A. Base: All pieces White (except origin)
                     for(int r=0; r<8; r++) {
                         for(int c=0; c<8; c++) {
                             if(board[r][c] != ' ' && !(r == originRow && c == originCol)) {
                                 boardDriver->setSquareLED(r, c, 50, 50, 50); // White
                             }
                         }
                     }
                     
                     // B. Legal Moves
                     for (int i = 0; i < moveCount; i++) {
                         int r = moves[i][0];
                         int c = moves[i][1];
                         if (board[r][c] == ' ') {
                             // Legal non-capture: White
                             boardDriver->setSquareLED(r, c, 50, 50, 50);
                         } else {
                             // Legal capture: Green
                             boardDriver->setSquareLED(r, c, 0, 255, 0);
                         }
                     }
                     
                     // C. Nearby Illegal (Red)
                     // Radius 1 around origin
                     for(int r = originRow - 1; r <= originRow + 1; r++) {
                         for(int c = originCol - 1; c <= originCol + 1; c++) {
                             if(r >= 0 && r < 8 && c >= 0 && c < 8) {
                                 // Skip origin itself
                                 if(r == originRow && c == originCol) continue;
                                 
                                 // Check if this square is in legal moves
                                 bool isLegal = false;
                                 for(int i=0; i<moveCount; i++) {
                                     if(moves[i][0] == r && moves[i][1] == c) {
                                         isLegal = true;
                                         break;
                                     }
                                 }
                                 
                                 if (!isLegal) {
                                     // Illegal nearby: Red
                                     // "Light red on illegal squares ... only those that ... are illegal"
                                     // Note: If a square is occupied by own piece (blocked), it IS illegal.
                                     // Logic: Red overrides White (base piece)? 
                                     // "Red LEDs on a subset of illegal squares... to give... hints"
                                     // Yes, make it Red.
                                     boardDriver->setSquareLED(r, c, 255, 0, 0); 
                                 }
                             }
                         }
                     }
                     
                     boardDriver->showLEDs();
                     boardDriver->readSensors();
                     
                     // Check for placement
                     // 1. Put back at origin?
                     if (boardDriver->getSensorState(originRow, originCol)) {
                         // Cancel
                         Serial.println("Placed back at origin");
                         placed = true;
                         // Effect: Will update LEDs next loop
                     }
                     
                     // 2. Placed elsewhere?
                     for(int r=0; r<8; r++) {
                         for(int c=0; c<8; c++) {
                             if (r == originRow && c == originCol) continue;
                             
                             // Detect NEW placement (High, was Low?) 
                             // Or just High. If it's High, and it wasn't origin...
                             // Note: Captures. If target had opponent, we expect it to be gone, then new piece placed.
                             // But 'getSensorState' is real-time. 
                             // If I place my piece, sensor is High.
                             // If I haven't removed opponent yet, sensor is High (opponent).
                             // If I remove opponent (Low), then place mine (High).
                             
                             // We only care if we detect a "Place Event". 
                             // Simple logic: If we find our piece at a NEW valid location.
                             // But how do we distinguish "Opponent Piece" from "My Piece Placed"?
                             // We don't have ID sensors.
                             // We assume if a square is Occupied, it's the piece that was there.
                             // UNLESS: 
                             // A. It was Empty, now Occupied -> Explicit Placement.
                             // B. It was Occupied (Capture), became Empty, then Occupied -> Explicit Placement.
                             // C. It was Occupied (Capture), and stays Occupied? 
                             //    - User physically jams piece on top? Unlikely.
                             //    - User removes opponent (Low), then places mine (High).
                             
                             // So: We look for a transition LOW -> HIGH on a square that isn't origin.
                             // OR: If target is Empty in board state, just HIGH is enough (since it was empty).
                             
                             bool isPlaceEvent = false;
                             
                             if (board[r][c] == ' ') {
                                 // Square logical empty: If Sensor High, it's a placement.
                                 if (boardDriver->getSensorState(r, c)) isPlaceEvent = true; 
                                 // (We might want debouncing or ensure it wasn't there before? 
                                 //  But 'board' says empty, so if sensor says full, it's new).
                             } else {
                                 // Square occupied (Capture):
                                 // We need to see it go LOW then HIGH? 
                                 // Or just rely on "SensorPrev" which is updated outside this loop? 
                                 // No, `boardDriver->updateSensorPrev()` is not called inside this loop!
                                 // So `getSensorPrev` is stuck at "Start of Update".
                                 
                                 // If `board` has piece, `SensorPrev` was likely HIGH.
                                 // If `Sensor` is now HIGH, it didn't change relative to start of frame.
                                 // To detect a capture placement:
                                 // We need local history inside this `while` loop.
                                 // Let's rely on: capture target MUST become empty first.
                             }
                             
                             // Simpler approach for Capture: 
                             // If target is in `legalMoves` (capture), and Sensor is HIGH...
                             // Did we see it go LOW?
                             // Maybe we just check: Is this a legal move?
                             // If yes, and Sensor is HIGH... is it the OLD piece or NEW one?
                             // We can't know for sure without state history.
                             // BUT: User instruction: "If a capture occurred...". 
                             // Let's implement: "If Valid Move & Square Empty & Sensor High" -> MOVED.
                             // "If Valid Capture... " -> Complex.
                             
                             // Let's use `checkPlacement` helper logic conceptually here.
                             // I will add a local `wasEmpty` array to track capture lifts?
                             // Too complex.
                             
                             // Pragmatic approach:
                             // If `board[r][c] == ' '` AND `sensor[r][c]` -> Placed.
                             // If `board[r][c] != ' '` (Capture):
                             //    If `sensor[r][c]` is LOW -> "Victim Lifted". (Visual? Green LED stays Green?)
                             //    If `sensor[r][c]` WAS LOW (detected locally) and NOW HIGH -> "Attacker Placed".
                         }
                     }
                      
                     // Simplified Detection Loop with Capture Support
                     // We monitor the board.
                     for(int r=0; r<8; r++) {
                         for(int c=0; c<8; c++) {
                             if(r == originRow && c == originCol) continue;
                             
                             // 1. Check for placement on Empty square (Move)
                             if(board[r][c] == ' ' && boardDriver->getSensorState(r, c)) {
                                 // Attempted Move to (r,c)
                                 bool legal = false;
                                 for(int i=0; i<moveCount; i++) if(moves[i][0] == r && moves[i][1] == c) legal = true;
                                 
                                 if(legal) {
                                     // Execute Move
                                     processMove(originRow, originCol, r, c, piece);
                                     boardDriver->setSquareLED(r, c, 50, 50, 50); // White
                                     boardDriver->showLEDs();
                                     placed = true;
                                 } else {
                                     // Illegal Placement
                                     boardDriver->setSquareLED(r, c, 255, 0, 0); // Red
                                     boardDriver->showLEDs();
                                     delay(800);
                                     // User must lift it again for us to continue highlighting
                                     // loop continues...
                                 }
                             }
                             
                             // 2. Check for placement on Occupied square (Capture)
                             // Logic: If we saw it go LOW, then HIGH.
                             // We need a local 'liftedTargets' set?
                             // Or just: If it is a Valid Capture Move, and the sensor 'flickers'?
                             // Actually, if the user picks up the victim, sensor goes LOW.
                             // We just need to track that.
                         }
                     }
                     
                     // Implementation of Capture State Tracking
                     static bool captureLifted[8][8]; // (Static is bad if re-entrant, use local)
                     // Reset captureLifted at start of pick? No.
                     // I will do a quick simpler logic:
                     // Iterate valid moves.
                     for(int i=0; i<moveCount; i++) {
                         int mr = moves[i][0];
                         int mc = moves[i][1];
                         if(board[mr][mc] != ' ') {
                             // This is a capture target.
                             // If sensor is LOW, it means victim removed.
                             if(!boardDriver->getSensorState(mr, mc)) {
                                 // Victim is gone. Now waiting for placement.
                                 // If it goes HIGH, and was LOW...
                                 // We can just say: If Sensor != Board (Board says occupied, Sensor says Empty)
                                 // Any future HIGH is the new piece.
                                 // Wait, if it goes HIGH, `board[mr][mc]` is still occupied (logic).
                                 // So `boardDriver->getSensorState` is HIGH.
                                 // How to distinguish from "Original piece never touched"?
                                 // We need `prevSensorState` relative to this drag interaction.
                             }
                         }
                     }
                     
                     // BETTER:
                     // Check specific sensor conditions for Legal Moves
                     for(int i=0; i<moveCount; i++) {
                         int mr = moves[i][0];
                         int mc = moves[i][1];
                         
                         if(board[mr][mc] == ' ') {
                             // Empty target: If Sensor HIGH -> Move
                             if(boardDriver->getSensorState(mr, mc)) {
                                 processMove(originRow, originCol, mr, mc, piece);
                                 boardDriver->setSquareLED(mr, mc, 50, 50, 50);
                                 boardDriver->showLEDs();
                                 placed = true;
                                 goto end_pick_loop;
                             }
                         } else {
                             // Capture target:
                             // To capture, one must lift the old piece (Sensor LOW) and place new (Sensor HIGH).
                             // If Sensor is HIGH, it might be the old piece.
                             // We only process if we SAW it go LOW?
                             // Or we check `!boardDriver->getSensorPrev(mr,mc)`? No, that's from before drag.
                             
                             // Let's use a very specific check:
                             // If (ValidCapture) AND (Sensor LOW) -> We know victim is lifted.
                             // Then we loop waiting for HIGH at `mr, mc`.
                             // But we need to keep rendering the board.
                             
                             if (!boardDriver->getSensorState(mr, mc)) { 
                                 // Piece removed!
                                 // Blink/Wait for placement logic or just continue loop allowing replacement?
                                 // If we just continue, next frame Sensor might be HIGH.
                                 // If Sensor HIGH and Board Occupied -> Ambiguous?
                                 // No, because we have a flag `captureReady[mr][mc]`.
                                 // But we can't allocate array.
                                 
                                 // Let's block-wait for the capture placement? 
                                 // "Update in real-time if board state changes".
                                 // If I block, I stop updating other LEDs.
                                 // But capture is a focused action. 
                                 // Let's try: If victim lifted, we light that square specifically (maybe blink Green?)
                                 // and wait for placement.
                                 
                                 boardDriver->setSquareLED(mr, mc, 0, 255, 0); // Solid Green
                                 boardDriver->showLEDs();
                                 
                                 // Quick wait for placement
                                 unsigned long capStart = millis();
                                 while(millis() - capStart < 2000) { // 2 sec timeout to prevent lock
                                     boardDriver->readSensors();
                                     if(boardDriver->getSensorState(mr, mc)) {
                                         // Placed!
                                         processMove(originRow, originCol, mr, mc, piece);
                                         // Optional flash green
                                         boardDriver->setSquareLED(mr, mc, 0, 255, 0); 
                                         boardDriver->showLEDs(); 
                                         delay(200);
                                         boardDriver->setSquareLED(mr, mc, 50, 50, 50);
                                         boardDriver->showLEDs(); 
                                         placed = true;
                                         goto end_pick_loop;
                                     }
                                     if(boardDriver->getSensorState(originRow, originCol)) {
                                         // Put back at origin (Cancel capture)
                                         placed = true; // Will execute "Placed back" logic next loop outer
                                         goto end_pick_loop;
                                     }
                                 }
                             }
                         }
                     }
                     
                     // Check Illegal Placements (Empty squares that are NOT in moves)
                     for(int r=0; r<8; r++) {
                         for(int c=0; c<8; c++) {
                             if(r==originRow && c==originCol) continue;
                             if(board[r][c] != ' ') continue; // Skip occupied
                             
                             if(boardDriver->getSensorState(r, c)) {
                                 // Placed on empty square check
                                 bool legal = false;
                                 for(int i=0; i<moveCount; i++) if(moves[i][0] == r && moves[i][1] == c) legal = true;
                                 
                                 if(!legal) {
                                     boardDriver->setSquareLED(r, c, 255, 0, 0);
                                     boardDriver->showLEDs();
                                     delay(800);
                                     // Loop continues, LED will be overwritten next frame
                                 }
                             }
                         }
                     }
                     
                     delay(50);
                 }
                 
                 end_pick_loop:
                 // Update LEDs for Idle
                 boardDriver->clearAllLEDs();
                 for(int r=0; r<8; r++) {
                     for(int c=0; c<8; c++) {
                         if(board[r][c] != ' ') {
                             boardDriver->setSquareLED(r, c, 50, 50, 50); // White
                             }
                     }
                 }
                 boardDriver->showLEDs();
             }
        }
    }
    
    boardDriver->updateSensorPrev();
}


void ChessMoves::initializeBoard() {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = INITIAL_BOARD[row][col];
        }
    }
}

void ChessMoves::waitForBoardSetup() {
    Serial.println("Waiting for pieces to be placed...");
    while (!boardDriver->checkInitialBoard(INITIAL_BOARD)) {
        boardDriver->updateSetupDisplay(INITIAL_BOARD);
        boardDriver->printBoardState(INITIAL_BOARD);
        delay(500);
    }
}

void ChessMoves::processMove(int fromRow, int fromCol, int toRow, int toCol, char piece) {
    // Update board state
    board[toRow][toCol] = piece;
    board[fromRow][fromCol] = ' ';
}

void ChessMoves::checkForPromotion(int targetRow, int targetCol, char piece) {
    if (chessEngine->isPawnPromotion(piece, targetRow)) {
        char promotedPiece = chessEngine->getPromotedPiece(piece);
        
        Serial.print((piece == 'P' ? "White" : "Black"));
        Serial.print(" pawn promoted to Queen at ");
        Serial.print((char)('a' + targetCol));
        Serial.println((piece == 'P' ? "8" : "1"));
        
        // Play promotion animation
        boardDriver->promotionAnimation(targetCol);
        
        // Promote to queen in board state
        board[targetRow][targetCol] = promotedPiece;
        
        // Handle the promotion process
        handlePromotion(targetRow, targetCol, piece);
    }
}

void ChessMoves::handlePromotion(int targetRow, int targetCol, char piece) {
    Serial.println("Please replace the pawn with a queen piece");
    
    // First wait for the pawn to be removed
    while (boardDriver->getSensorState(targetRow, targetCol)) {
        // Blink the square to indicate action needed
        boardDriver->setSquareLED(targetRow, targetCol, 255, 215, 0, 50);
        boardDriver->showLEDs();
        delay(250);
        boardDriver->setSquareLED(targetRow, targetCol, 0, 0, 0, 0);
        boardDriver->showLEDs();
        delay(250);
        
        // Read sensors
        boardDriver->readSensors();
    }
    
    Serial.println("Pawn removed, please place a queen");
    
    // Then wait for the queen to be placed
    while (!boardDriver->getSensorState(targetRow, targetCol)) {
        // Blink the square to indicate action needed
        boardDriver->setSquareLED(targetRow, targetCol, 255, 215, 0, 50);
        boardDriver->showLEDs();
        delay(250);
        boardDriver->setSquareLED(targetRow, targetCol, 0, 0, 0, 0);
        boardDriver->showLEDs();
        delay(250);
        
        // Read sensors
        boardDriver->readSensors();
    }
    
    Serial.println("Queen placed, promotion complete");
    
    // Final confirmation blink
    for (int i = 0; i < 3; i++) {
        boardDriver->setSquareLED(targetRow, targetCol, 255, 215, 0, 50);
        boardDriver->showLEDs();
        delay(100);
        boardDriver->setSquareLED(targetRow, targetCol, 0, 0, 0, 0);
        boardDriver->showLEDs();
        delay(100);
    }
}

bool ChessMoves::isActive() {
    return true; // Simple implementation for now
}

void ChessMoves::reset() {
    boardDriver->clearAllLEDs();
    initializeBoard();
}