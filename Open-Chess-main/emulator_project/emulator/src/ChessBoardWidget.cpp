#include "ChessBoardWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

ChessBoardWidget::ChessBoardWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    dragging = false;
    dragSourceRow = -1;
    dragSourceCol = -1;
    dragPiece = ' ';

    // Initialize board (Makruk) - copying chess_moves.cpp
    // White
    const char wBase[] = "RNBQKBNR";
    for(int i=0; i<8; i++) board[0][i] = wBase[i];
    for(int i=0; i<8; i++) board[1][i] = ' ';
    for(int i=0; i<8; i++) board[2][i] = 'P';
    for(int i=0; i<8; i++) board[3][i] = ' ';
    for(int i=0; i<8; i++) board[4][i] = ' ';
    // Black
    for(int i=0; i<8; i++) board[5][i] = 'p';
    for(int i=0; i<8; i++) board[6][i] = ' ';
    const char bBase[] = "rnbkqbnr"; // k at 3, q at 4
    for(int i=0; i<8; i++) board[7][i] = bBase[i];

    // Clear LEDs
    for(int r=0; r<8; r++)
        for(int c=0; c<8; c++)
            ledColors[r][c] = Qt::transparent;
            
    setMinimumSize(400, 400);
}

void ChessBoardWidget::setLed(int r, int c, QColor color) {
    if(r >=0 && r<8 && c>=0 && c<8) {
        ledColors[r][c] = color;
    }
}

void ChessBoardWidget::clearLeds() {
    for(int r=0; r<8; r++)
        for(int c=0; c<8; c++)
            ledColors[r][c] = Qt::transparent;
}

void ChessBoardWidget::showLeds() {
    update();
}

int ChessBoardWidget::getCellSize() const {
    return std::min(width(), height()) / 8;
}

void ChessBoardWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int s = getCellSize();

    // Draw board
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
             // Rank 0 is bottom. Visual Row 7 is internal row 0?
             // No, usually Visual (0,0) is Top-Left (Rank 8, File A).
             // Internal Row 0 = Rank 1.
             // So Internal Row 0 should be at Visual Y = 7*s.
             // ScreenY = (7 - r) * s.
             // ScreenX = c * s.
             int x = c * s;
             int y = (7 - r) * s;
             
             // Checkboard pattern
             if ((r + c) % 2 == 1) p.fillRect(x, y, s, s, QColor(200, 200, 200)); // Light
             else p.fillRect(x, y, s, s, QColor(100, 100, 100)); // Dark (Actually green/wood usually)

             // Draw LED if active
             if (ledColors[r][c] != Qt::transparent && ledColors[r][r].alpha() != 0) {
                 // Draw halo/ring
                 p.setPen(QPen(ledColors[r][c], 4));
                 p.setBrush(Qt::NoBrush);
                 p.drawRect(x+2, y+2, s-4, s-4);
                 
                 // Or fill with alpha
                 QColor fill = ledColors[r][c];
                 fill.setAlpha(100);
                 p.fillRect(x, y, s, s, fill);
             }
             
             // Draw Piece
             char piece = board[r][c];
             if (dragging && r == dragSourceRow && c == dragSourceCol) {
                 // Don't draw piece here, it's being dragged
             } else {
                 if (piece != ' ') drawPiece(p, piece, x, y, s);
             }
        }
    }
    
    // Draw dragging piece
    if (dragging) {
        drawPiece(p, dragPiece, dragCurrentPos.x() - s/2, dragCurrentPos.y() - s/2, s);
    }
}

void ChessBoardWidget::drawPiece(QPainter& p, char piece, int x, int y, int size) {
    // Simple text for now, can be replaced by SVGs
    // Map chars to Unicode chess pieces?
    // Makruk doesn't have unicode. Uses standard chess pieces usually.
    // R=Rook, N=Knight, B=Bishop(Khon), Q=Queen(Met), K, P
    
    QString text;
    bool white = isupper(piece);
    char pLow = tolower(piece);
    
    // Use standard chess unicode
    if (white) {
        switch(pLow) {
            case 'k': text = "♔"; break; 
            case 'q': text = "♕"; break; // Met
            case 'r': text = "♖"; break;
            case 'b': text = "♗"; break; // Khon
            case 'n': text = "♘"; break;
            case 'p': text = "♙"; break; // Bia
        }
    } else {
        switch(pLow) {
            case 'k': text = "♚"; break;
            case 'q': text = "♛"; break;
            case 'r': text = "♜"; break;
            case 'b': text = "♝"; break;
            case 'n': text = "♞"; break;
            case 'p': text = "♟"; break;
        }
    }
    
    QFont font = p.font();
    font.setPixelSize(size * 0.8);
    p.setFont(font);
    p.setPen(white ? Qt::white : Qt::black);
    // Draw outline for visibility
    if (!white) p.setPen(Qt::black);
    else p.setPen(Qt::white);
    
    // Center text
    p.drawText(x, y, size, size, Qt::AlignCenter, text);
}

void ChessBoardWidget::mousePressEvent(QMouseEvent *event) {
    int s = getCellSize();
    int c = event->pos().x() / s;
    int r = 7 - (event->pos().y() / s); // Flip Y
    
    if (r>=0 && r<8 && c>=0 && c<8) {
        if (board[r][c] != ' ') {
            dragging = true;
            dragSourceRow = r;
            dragSourceCol = c;
            dragPiece = board[r][c];
            dragCurrentPos = event->pos();
            
            // Emit Lift Event
            emit sensorChanged(r, c, false); // Sensor goes LOW (Empty)
            
            update();
        }
    }
}

void ChessBoardWidget::mouseMoveEvent(QMouseEvent *event) {
    if (dragging) {
        dragCurrentPos = event->pos();
        update();
    }
}

void ChessBoardWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (dragging) {
        int s = getCellSize();
        int c = event->pos().x() / s;
        int r = 7 - (event->pos().y() / s);
        
        // Remove from old
        board[dragSourceRow][dragSourceCol] = ' ';
        
        // Place in new
        if (r>=0 && r<8 && c>=0 && c<8) {
            // Overwrite if exists (Capture logic in GUI is destructive)
            board[r][c] = dragPiece;
            
            // Emit Drop Event
             emit sensorChanged(r, c, true); // Sensor goes HIGH (Occupied)
        } else {
            // Dropped off board? Return to start?
            // "Piece lifted". If dropped off board, sensor at Source is now Empty.
            // But we didn't put it anywhere.
            // Firmware will think piece is "in hand".
            // If we want to cancel, we should put it back.
            board[dragSourceRow][dragSourceCol] = dragPiece;
            emit sensorChanged(dragSourceRow, dragSourceCol, true);
        }
        
        dragging = false;
        update();
    }
}
