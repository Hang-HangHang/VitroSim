#include "datareceiver.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DataReceiver::DataReceiver(QObject *parent)
    : QObject(parent), m_isConnected(false)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &DataReceiver::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &DataReceiver::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &DataReceiver::onReadyRead);
}

void DataReceiver::connectToDevice(const QString &host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

void DataReceiver::onConnected()
{
    m_isConnected = true;
    emit connectionStatus("✅ 已连接虚拟光度计 (多通道 JSON)");
}

void DataReceiver::onDisconnected()
{
    m_isConnected = false;
    emit connectionStatus("⚠️ 虚拟光度计断开");
}

void DataReceiver::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    parseBuffer();
}

void DataReceiver::parseBuffer()
{
    int index;
    while ((index = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(index);
        m_buffer.remove(0, index + 1);

        // 打印原始行（方便调试）
        qDebug() << "收到原始行:" << line;

        if (line.startsWith('{')) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(line, &error);
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("type") && obj["type"] == "multi_od") {
                    if (obj.contains("channels") && obj["channels"].isArray()) {
                        QJsonArray arr = obj["channels"].toArray();
                        QList<double> values;
                        for (const auto& v : arr) {
                            values.append(v.toDouble());
                        }
                        if (values.size() == 32) {
                            qDebug() << "解析成功，32个通道数据";
                            emit multiOdReceived(values);
                            continue;
                        } else {
                            qDebug() << "通道数量不是32，实际为:" << values.size();
                        }
                    } else {
                        qDebug() << "缺少 channels 字段或不是数组";
                    }
                } else {
                    qDebug() << "type 不是 multi_od";
                }
            } else {
                qDebug() << "JSON解析失败:" << error.errorString();
            }
        } else {
            qDebug() << "行不以 { 开头，忽略";
        }
    }
}