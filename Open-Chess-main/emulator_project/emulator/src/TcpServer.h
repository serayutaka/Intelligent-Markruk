#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public QObject {
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    void start(int port = 2323);
    void sendSensor(int r, int c, bool pressed);

signals:
    void setLed(int r, int c, int red, int green, int blue);
    void clearLeds();
    void showLeds();

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTcpServer *server;
    QTcpSocket *client;
};
