#include "deviceserver.h"
#include <QDebug>

DeviceServer::DeviceServer(QObject *parent) : QObject(parent), m_server(nullptr) {}

void DeviceServer::startServer(quint16 port) {
    if (m_server) return;
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &DeviceServer::onNewConnection);
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "虚拟设备服务器已启动，端口:" << port;
    } else {
        qDebug() << "启动失败:" << m_server->errorString();
    }
}

void DeviceServer::onNewConnection() {
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;
    m_clients.append(socket);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &DeviceServer::onClientDisconnected);
    socket->write("Connected to VitroSim Simulator\r\n");
    qDebug() << "虚拟设备已连接";
}

void DeviceServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    QByteArray data = socket->readAll();
    QString cmd = QString::fromUtf8(data).trimmed();
    emit commandReceived(cmd);  // 发给主窗口处理
}

void DeviceServer::onClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_clients.removeAll(socket);
        socket->deleteLater();
        qDebug() << "虚拟设备断开";
    }
}