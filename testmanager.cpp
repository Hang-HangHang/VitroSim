#include "testmanager.h"
#include <QDebug>
#include "logger.h"

TestManager::TestManager(QObject *parent)
    : QObject(parent)
    , m_globalState(Idle)
    , m_step(0)
    , m_maxPoints(30)
    , m_activeChannel(0)
    , m_testCompleted(false)
    , m_globalPointCount(0)
{
    initChannels();
}

void TestManager::initChannels()
{
    for (int i = 1; i <= 32; ++i) {
        ChannelData data;
        data.state = Idle;
        m_channels[i] = data;
    }
}

void TestManager::start(const QList<int>& channels)
{
    if (m_globalState != Idle) return;

    m_testCompleted = false;
    m_globalPointCount = 0;

    if (channels.isEmpty()) {
        for (int i = 1; i <= 32; ++i) m_activeChannels.append(i);
    } else {
        m_activeChannels = channels;
    }

    m_globalState = Preheating;
    m_step = 0;
    initChannels();

    emit stateChanged(m_globalState);
    emit logMessage(QString("🔥 开始预热，选中 %1 个通道...").arg(m_activeChannels.size()));

    QTimer::singleShot(2000, this, &TestManager::goToNextStep);
}

void TestManager::goToNextStep()
{
    m_step++;

    if (m_step == 1) {
        m_globalState = Testing;
        for (int ch : m_activeChannels) {
            m_channels[ch].state = Testing;
            emit channelStateChanged(ch, Testing);
        }
        emit stateChanged(m_globalState);
        emit logMessage(QString("🌡️ 温度稳定，开始采集（共 %1 个点）...").arg(m_maxPoints));

    } else if (m_step == 2) {
        // 由 completeAllChannels 触发，直接进入计算
        m_globalState = Calculating;
        emit stateChanged(m_globalState);
        emit logMessage("🧪 正在计算浓度...");

        for (int ch : m_activeChannels) {
            ChannelData& data = m_channels[ch];
            if (!data.odValues.isEmpty()) {
                double sum = 0;
                for (double v : data.odValues) sum += v;
                data.avgOd = sum / data.odValues.size();
                data.isCompleted = true;
            }
        }

        QTimer::singleShot(1500, this, &TestManager::goToNextStep);

    } else if (m_step == 3) {
        m_globalState = Complete;
        m_testCompleted = true;
        emit stateChanged(m_globalState);
        emit logMessage("✅ 测试完成！");

        for (int ch : m_activeChannels) {
            if (m_channels[ch].isCompleted) {
                emit channelCompleted(ch, m_channels[ch].avgOd, m_channels[ch].concentration);
            }
        }
    }
}

void TestManager::addFrame(const QList<double>& values)
{
    if (m_globalState != Testing) return;
    if (m_testCompleted) return;

    // 确保 values 长度至少覆盖活动通道
    if (values.size() < 32) return;

    // 递增全局计数器
    m_globalPointCount++;

    // 对所有活动通道追加数据
    for (int ch : m_activeChannels) {
        ChannelData& data = m_channels[ch];
        // 防止已经完成
        if (data.isCompleted) continue;
        data.odValues.append(values[ch - 1]);
        data.state = Testing;
    }

    // 发出全局进度信号
    emit globalProgress(m_globalPointCount, m_maxPoints);

    // 检查是否达到最大点数
    if (m_globalPointCount >= m_maxPoints) {
        completeAllChannels();
    }
}

void TestManager::completeAllChannels()
{
    if (m_globalState != Testing) return;
    if (m_testCompleted) return;

    // 标记所有活动通道为完成
    for (int ch : m_activeChannels) {
        ChannelData& data = m_channels[ch];
        if (!data.isCompleted) {
            data.isCompleted = true;
            double sum = 0;
            for (double v : data.odValues) sum += v;
            data.avgOd = sum / data.odValues.size();
            emit channelStateChanged(ch, Complete);
            emit logMessage(QString("通道 %1 采集完成，点数: %2").arg(ch).arg(data.odValues.size()));
        }
    }

    // 进入计算步骤（跳过定时器等待）
    if (m_step == 1) {
        m_step = 2;
        goToNextStep();
    }
}

void TestManager::pause()
{
    if (m_globalState == Preheating || m_globalState == Testing || m_globalState == Calculating) {
        m_globalState = Error;
        emit stateChanged(m_globalState);
        emit logMessage("⚠️ 紧急暂停！");
    }
}

void TestManager::resetAll()
{
    m_globalState = Idle;
    m_step = 0;
    m_activeChannel = 0;
    m_activeChannels.clear();
    m_testCompleted = false;
    m_globalPointCount = 0;
    initChannels();
    emit stateChanged(m_globalState);
    emit logMessage("🔄 已复位");
}

void TestManager::resetChannel(int channel)
{
    if (m_channels.contains(channel)) {
        ChannelData& data = m_channels[channel];
        data.odValues.clear();
        data.avgOd = 0;
        data.concentration = 0;
        data.isCompleted = false;
        data.isError = false;
        data.state = Idle;
        emit channelStateChanged(channel, Idle);
    }
}

double TestManager::getAvgOd(int channel) const
{
    if (!m_channels.contains(channel)) return 0;
    return m_channels[channel].avgOd;
}

QList<double> TestManager::getOdList(int channel) const
{
    if (!m_channels.contains(channel)) return QList<double>();
    return m_channels[channel].odValues;
}

int TestManager::getPointCount(int channel) const
{
    if (!m_channels.contains(channel)) return 0;
    return m_channels[channel].odValues.size();
}

ChannelData TestManager::getChannelData(int channel) const
{
    if (!m_channels.contains(channel)) return ChannelData();
    return m_channels[channel];
}

bool TestManager::allChannelsCompleted() const
{
    for (int ch : m_activeChannels) {
        if (!m_channels[ch].isCompleted) return false;
    }
    return true;
}