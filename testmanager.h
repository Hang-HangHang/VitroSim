#ifndef TESTMANAGER_H
#define TESTMANAGER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QMap>

enum State {
    Idle,
    Preheating,
    Testing,
    Calculating,
    Complete,
    Error
};

struct ChannelData {
    QList<double> odValues;
    double avgOd;
    double concentration;
    bool isCompleted;
    bool isError;
    State state;

    ChannelData() : avgOd(0), concentration(0), isCompleted(false), isError(false), state(Idle) {}
};

class TestManager : public QObject
{
    Q_OBJECT
public:
    explicit TestManager(QObject *parent = nullptr);

    // 启动测试（传入选中的通道列表）
    void start(const QList<int>& channels = QList<int>());
    void pause();
    void resetAll();
    void resetChannel(int channel);

    State getState() const { return m_globalState; }
    bool isTestCompleted() const { return m_testCompleted; }

    // 新增：一次性处理一帧所有通道数据（所有通道同时推进）
    void addFrame(const QList<double>& values);

    // 单个通道查询接口
    double getAvgOd(int channel) const;
    QList<double> getOdList(int channel) const;
    int getPointCount(int channel) const;
    int getMaxPoints() const { return m_maxPoints; }

    ChannelData getChannelData(int channel) const;
    QMap<int, ChannelData> getAllChannelData() const { return m_channels; }

    int getActiveChannel() const { return m_activeChannel; }
    void setActiveChannel(int channel) { m_activeChannel = channel; }

    QList<int> getActiveChannels() const { return m_activeChannels; }
    void setActiveChannels(const QList<int>& channels) { m_activeChannels = channels; }

signals:
    void stateChanged(State newState);
    void logMessage(QString msg);
    void channelStateChanged(int channel, State state);
    void channelCompleted(int channel, double avgOd, double concentration);
    void globalProgress(int current, int total);   // 全局采集进度

private slots:
    void goToNextStep();

private:
    State m_globalState;
    int m_step;
    int m_maxPoints;
    int m_activeChannel;
    QList<int> m_activeChannels;
    bool m_testCompleted;

    // 全局采集计数器
    int m_globalPointCount;

    QMap<int, ChannelData> m_channels;

    void initChannels();
    bool allChannelsCompleted() const;
    void completeAllChannels();   // 完成所有通道
};

#endif // TESTMANAGER_H