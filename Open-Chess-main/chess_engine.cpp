#include "chess_engine.h"
#include <Arduino.h>

// ---------------------------
// ChessEngine Implementation
// ---------------------------

ChessEngine::ChessEngine() {
    // Constructor - nothing to initialize for now
}

// Main move generation function
void ChessEngine::getPossibleMoves(const char board[8][8], int row, int col, int &moveCount, int moves[][2]) {
    moveCount = 0;
    char piece = board[row][col];
    
    if (piece == ' ') return; // Empty square
    
    char pieceColor = getPieceColor(piece);
    
    // Convert to uppercase for easier comparison
    piece = (piece >= 'a' && piece <= 'z') ? piece - 32 : piece;

    switch(piece) {
        case 'P': // Pawn
            addPawnMoves(board, row, col, pieceColor, moveCount, moves);
            break;
        case 'R': // Rook
            addRookMoves(board, row, col, pieceColor, moveCount, moves);
            break;
        case 'N': // Knight
            addKnightMoves(board, row, col, pieceColor, moveCount, moves);
            break;
        case 'B': // Bishop
            addBishopMoves(board, row, col, pieceColor, moveCount, moves);
            break;
        case 'Q': // Queen
            addQueenMoves(board, row, col, pieceColor, moveCount, moves);
            break;
        case 'K': // King
            addKingMoves(board, row, col, pieceColor, moveCount, moves);
            break;
    }
}

// Pawn move generation (Makruk Rules: Bia)
void ChessEngine::addPawnMoves(const char board[8][8], int row, int col, char pieceColor, int &moveCount, int moves[][2]) {
    int direction = (pieceColor == 'w') ? 1 : -1;
    
    // One square forward
    if (isValidSquare(row + direction, col) && isSquareEmpty(board, row + direction, col)) {
        moves[moveCount][0] = row + direction;
        moves[moveCount][1] = col;
        moveCount++;
    }
    
    // Diagonal captures
    int captureColumns[] = {col-1, col+1};
    for (int i = 0; i < 2; i++) {
        int captureRow = row + direction;
        int captureCol = captureColumns[i];
        
        if (isValidSquare(captureRow, captureCol) && 
            isSquareOccupiedByOpponent(board, captureRow, captureCol, pieceColor)) {
            moves[moveCount][0] = captureRow;
            moves[moveCount][1] = captureCol;
            moveCount++;
        }
    }
}

// Rook move generation (Makruk Rules: Ruea - Same as Chess)
void ChessEngine::addRookMoves(const char board[8][8], int row, int col, char pieceColor, int &moveCount, int moves[][2]) {
    int directions[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
    
    for (int d = 0; d < 4; d++) {
        for (int step = 1; step < 8; step++) {
            int newRow = row + step * directions[d][0];
            int newCol = col + step * directions[d][1];
            
            if (!isValidSquare(newRow, newCol)) break;
            
            if (isSquareEmpty(board, newRow, newCol)) {
                moves[moveCount][0] = newRow;
                moves[moveCount][1] = newCol;
                moveCount++;
            } else {
                // Check if it's a capturable piece
                if (isSquareOccupiedByOpponent(board, newRow, newCol, pieceColor)) {
                    moves[moveCount][0] = newRow;
                    moves[moveCount][1] = newCol;
                    moveCount++;
                }
                break; // Can't move past any piece
            }
        }
    }
}

// Knight move generation (Makruk Rules: Ma - Same as Chess)
void ChessEngine::addKnightMoves(const char board[8][8], int row, int col, char pieceColor, int &moveCount, int moves[][2]) {
    int knightMoves[8][2] = {{2,1}, {1,2}, {-1,2}, {-2,1},
                             {-2,-1}, {-1,-2}, {1,-2}, {2,-1}};
    
    for (int i = 0; i < 8; i++) {
        int newRow = row + knightMoves[i][0];
        int newCol = col + knightMoves[i][1];
        
        if (isValidSquare(newRow, newCol)) {
            if (isSquareEmpty(board, newRow, newCol) || 
                isSquareOccupiedByOpponent(board, newRow, newCol, pieceColor)) {
                moves[moveCount][0] = newRow;
                moves[moveCount][1] = newCol;
                moveCount++;
            }
        }
    }
}

// Bishop move generation (Makruk Rules: Khon)
void ChessEngine::addBishopMoves(const char board[8][8], int row, int col, char pieceColor, int &moveCount, int moves[][2]) {
    // Khon moves: 1 step diagonal in any direction, OR 1 step forward
    int forwardDir = (pieceColor == 'w') ? 1 : -1;

    // Diagonal moves (1 step)
    int diagMoves[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
    for (int i = 0; i < 4; i++) {
        int newRow = row + diagMoves[i][0];
        int newCol = col + diagMoves[i][1];
        if (isValidSquare(newRow, newCol)) {
            if (isSquareEmpty(board, newRow, newCol) || isSquareOccupiedByOpponent(board, newRow, newCol, pieceColor)) {
                moves[moveCount][0] = newRow;
                moves[moveCount][1] = newCol;
                moveCount++;
            }
        }
    }

    // Forward move (1 step)
    int fwdRow = row + forwardDir;
    int fwdCol = col;
    if (isValidSquare(fwdRow, fwdCol)) {
        if (isSquareEmpty(board, fwdRow, fwdCol) || isSquareOccupiedByOpponent(board, fwdRow, fwdCol, pieceColor)) {
            moves[moveCount][0] = fwdRow;
            moves[moveCount][1] = fwdCol;
            moveCount++;
        }
    }
}

// Queen move generation (Makruk Rules: Met)
void ChessEngine::addQueenMoves(const char board[8][8], int row, int col, char pieceColor, int &moveCount, int moves[][2]) {
    // Met moves: 1 step diagonal in any direction (4 squares max)
    int movesList[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
    
    for (int i = 0; i < 4; i++) {
        int newRow = row + movesList[i][0];
        int newCol = col + movesList[i][1];
        
        if (isValidSquare(newRow, newCol)) {
             if (isSquareEmpty(board, newRow, newCol) || isSquareOccupiedByOpponent(board, newRow, newCol, pieceColor)) {
                moves[moveCount][0] = newRow;
                moves[moveCount][1] = newCol;
                moveCount++;
            }
        }
    }
}


// King move generation
void ChessEngine::addKingMoves(const char board[8][8], int row, int col, char pieceColor, int &moveCount, int moves[][2]) {
    int kingMoves[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1},
                           {1,1}, {1,-1}, {-1,1}, {-1,-1}};
    
    for (int i = 0; i < 8; i++) {
        int newRow = row + kingMoves[i][0];
        int newCol = col + kingMoves[i][1];
        
        if (isValidSquare(newRow, newCol)) {
            if (isSquareEmpty(board, newRow, newCol) || 
                isSquareOccupiedByOpponent(board, newRow, newCol, pieceColor)) {
                moves[moveCount][0] = newRow;
                moves[moveCount][1] = newCol;
                moveCount++;
            }
        }
    }
}

// Helper function to check if a square is occupied by an opponent piece
bool ChessEngine::isSquareOccupiedByOpponent(const char board[8][8], int row, int col, char pieceColor) {
    char targetPiece = board[row][col];
    if (targetPiece == ' ') return false;
    
    char targetColor = getPieceColor(targetPiece);
    return targetColor != pieceColor;
}

// Helper function to check if a square is empty
bool ChessEngine::isSquareEmpty(const char board[8][8], int row, int col) {
    return board[row][col] == ' ';
}

// Helper function to check if coordinates are within board bounds
bool ChessEngine::isValidSquare(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

// Helper function to get piece color
char ChessEngine::getPieceColor(char piece) {
    return (piece >= 'a' && piece <= 'z') ? 'b' : 'w';
}

// Move validation
bool ChessEngine::isValidMove(const char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    int moveCount = 0;
    int moves[28][2]; // Maximum possible moves for a queen
    
    getPossibleMoves(board, fromRow, fromCol, moveCount, moves);
    
    for (int i = 0; i < moveCount; i++) {
        if (moves[i][0] == toRow && moves[i][1] == toCol) {
            return true;
        }
    }
    return false;
}

// Check if a pawn move results in promotion (Makruk Rules)
bool ChessEngine::isPawnPromotion(char piece, int targetRow) {
    if (piece == 'P' && targetRow == 5) return true;  // White pawn reaches rank 6 (row 5)
    if (piece == 'p' && targetRow == 2) return true;  // Black pawn reaches rank 3 (row 2)
    return false;
}

// Get the promoted piece (Makruk: Promotes to Met/Queen)
char ChessEngine::getPromotedPiece(char piece) {
    return (piece == 'P') ? 'Q' : 'q';
}

// Utility function to print a move in readable format
void ChessEngine::printMove(int fromRow, int fromCol, int toRow, int toCol) {
    Serial.print((char)('a' + fromCol));
    Serial.print(fromRow + 1);
    Serial.print(" to ");
    Serial.print((char)('a' + toCol));
    Serial.println(toRow + 1);
}

// Convert algebraic notation file (a-h) to column index (0-7)
char ChessEngine::algebraicToCol(char file) {
    return file - 'a';
}

// Convert algebraic notation rank (1-8) to row index (0-7)
int ChessEngine::algebraicToRow(int rank) {
    return rank - 1;
}