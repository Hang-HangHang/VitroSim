#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

/**
 * @brief 日志系统单例类，用于记录程序运行过程中的关键事件、错误和警告。
 *
 * 日志文件保存在程序所在目录的 logs/ 文件夹中，文件名格式为：
 * VitroSim_YYYYMMDD_HHMMSS.log
 *
 * 每条日志格式：[时间] 级别: 消息
 * 级别包括：INFO、WARNING、ERROR
 *
 * 线程安全（使用 QMutex 保护文件写入）。
 */
class Logger : public QObject
{
    Q_OBJECT
public:
    /// 获取单例实例
    static Logger* instance();

    /// 记录普通信息（程序启动、状态切换、采集进度等）
    static void info(const QString &msg);

    /// 记录警告（如OD值稍高、连接中断等）
    static void warning(const QString &msg);

    /// 记录错误（如连接失败、文件写入失败等）
    static void error(const QString &msg);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    /// 实际写入日志的内部方法，加锁保护
    void writeLog(const QString &level, const QString &msg);

    QFile        m_file;         // 日志文件
    QTextStream  m_stream;       // 文本流
    QMutex       m_mutex;        // 互斥锁，保证线程安全
    bool         m_isOpen;       // 文件是否正常打开
};

#endif // LOGGER_H