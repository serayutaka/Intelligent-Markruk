#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include "ChessBoardWidget.h"
#include "TcpServer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onLog(const QString& msg);

private:
    ChessBoardWidget *boardWidget;
    TcpServer *server;
    QTextEdit *logView;
};
