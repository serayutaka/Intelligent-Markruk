#pragma once
#include <QWidget>
#include <QMap>
#include <QColor>

class ChessBoardWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChessBoardWidget(QWidget *parent = nullptr);
    void setLed(int r, int c, QColor color);
    void clearLeds();
    void showLeds(); // Triggers update

signals:
    void sensorChanged(int r, int c, bool pressed);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    char board[8][8];
    QColor ledColors[8][8];
    bool ledActive[8][8]; // Is there a pending LED update?
    
    // Dragging
    bool dragging;
    int dragSourceRow, dragSourceCol;
    QPoint dragCurrentPos;
    char dragPiece;
    
    int getCellSize() const;
    void drawPiece(QPainter& p, char piece, int x, int y, int size);
};
