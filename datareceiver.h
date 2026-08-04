#ifndef DATARECEIVER_H
#define DATARECEIVER_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>

class DataReceiver : public QObject
{
    Q_OBJECT
public:
    explicit DataReceiver(QObject *parent = nullptr);
    void connectToDevice(const QString &host, quint16 port);

    bool isConnected() const { return m_isConnected; }

signals:
    // 旧版单值信号（保留兼容，但新协议不再使用）
    void odValueReceived(double value);
    // 新版多通道信号
    void multiOdReceived(QList<double> values);
    void connectionStatus(QString status);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();

private:
    QTcpSocket *m_socket;
    QByteArray m_buffer;
    bool m_isConnected;
    void parseBuffer();
};

#endif // DATARECEIVER_H