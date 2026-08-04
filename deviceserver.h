#ifndef DEVICESERVER_H
#define DEVICESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class DeviceServer : public QObject
{
    Q_OBJECT
public:
    explicit DeviceServer(QObject *parent = nullptr);
    void startServer(quint16 port = 8888);

signals:
    void commandReceived(QString cmd);  // 收到指令，发给主界面

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
};

#endif