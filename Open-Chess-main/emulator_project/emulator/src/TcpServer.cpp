#include "TcpServer.h"
#include <QDebug>

TcpServer::TcpServer(QObject *parent) : QObject(parent), server(new QTcpServer(this)), client(nullptr) {}

void TcpServer::start(int port) {
    if(!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server could not start";
    } else {
        qDebug() << "Server started on port" << port;
        connect(server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    }
}

void TcpServer::onNewConnection() {
    if(client) {
        client->disconnectFromHost();
    }
    client = server->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater);
    qDebug() << "Client connected";
}

void TcpServer::onReadyRead() {
    if(!client) return;
    
    while(client->canReadLine()) {
        QByteArray line = client->readLine().trimmed();
        QString cmd = QString::fromUtf8(line);
        // Protocol: 
        // L r c r g b  (Set LED)
        // C (Clear)
        // S (Show)
        
        QStringList parts = cmd.split(' ');
        if(parts.isEmpty()) continue;
        
        QString type = parts[0];
        if(type == "C") {
            emit clearLeds();
        } else if(type == "S") {
            emit showLeds();
        } else if(type == "L" && parts.size() >= 6) {
            int r = parts[1].toInt();
            int c = parts[2].toInt();
            int red = parts[3].toInt();
            int grn = parts[4].toInt();
            int blu = parts[5].toInt();
            emit setLed(r, c, red, grn, blu);
        }
    }
}

void TcpServer::sendSensor(int r, int c, bool pressed) {
    if(client && client->state() == QAbstractSocket::ConnectedState) {
        // Protocol: E <row> <col> <1|0>
        QString msg = QString("E %1 %2 %3\n").arg(r).arg(c).arg(pressed ? 1 : 0);
        client->write(msg.toUtf8());
        client->flush();
    }
}
