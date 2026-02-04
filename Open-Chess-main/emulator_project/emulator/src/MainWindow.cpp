#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    
    // Left: Board
    QVBoxLayout *leftLayout = new QVBoxLayout();
    boardWidget = new ChessBoardWidget(this);
    leftLayout->addWidget(new QLabel("<h2>Virtual Makruk Board</h2>"));
    leftLayout->addWidget(boardWidget);
    leftLayout->addStretch();
    mainLayout->addLayout(leftLayout, 2);
    
    // Right: Controls & Log
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    QGroupBox *logGroup = new QGroupBox("Communication Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    logView = new QTextEdit();
    logView->setReadOnly(true);
    logLayout->addWidget(logView);
    rightLayout->addWidget(logGroup);
    
    mainLayout->addLayout(rightLayout, 1);
    
    resize(1000, 600);
    setWindowTitle("OpenChess Makruk Emulator");
    
    // Server Setup
    server = new TcpServer(this);
    
    // Connect Server -> Board (Outputs)
    connect(server, &TcpServer::setLed, [this](int r, int c, int red, int g, int b){
        boardWidget->setLed(r, c, QColor(red, g, b));
        onLog(QString("LED [%1,%2] -> R%3 G%4 B%5").arg(r).arg(c).arg(red).arg(g).arg(b));
    });
    connect(server, &TcpServer::clearLeds, [this](){
        boardWidget->clearLeds();
        // onLog("LEDs Cleared"); // verbose
    });
    connect(server, &TcpServer::showLeds, boardWidget, &ChessBoardWidget::showLeds);
    
    // Connect Board -> Server (Inputs)
    connect(boardWidget, &ChessBoardWidget::sensorChanged, [this](int r, int c, bool pressed){
        server->sendSensor(r, c, pressed);
        onLog(QString("Sensor [%1,%2] -> %3").arg(r).arg(c).arg(pressed ? "Occupied" : "Empty"));
    });
    
    server->start();
    onLog("Emulator started. Listening on port 2323...");
    onLog("Waiting for firmware connection...");
}

void MainWindow::onLog(const QString& msg) {
    logView->append(QString("[%1] %2").arg(QTime::currentTime().toString(), msg));
}
