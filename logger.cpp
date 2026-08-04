#include "logger.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

Logger* Logger::instance()
{
    // 静态单例，程序结束时自动销毁
    static Logger logger;
    return &logger;
}

void Logger::info(const QString &msg)
{
    instance()->writeLog("INFO", msg);
}

void Logger::warning(const QString &msg)
{
    instance()->writeLog("WARNING", msg);
}

void Logger::error(const QString &msg)
{
    instance()->writeLog("ERROR", msg);
}

Logger::Logger(QObject *parent)
    : QObject(parent), m_isOpen(false)
{
    // 创建 logs 目录（如果不存在）
    QString logDirPath = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir;
    if (!dir.exists(logDirPath)) {
        dir.mkpath(logDirPath);
    }

    // 生成日志文件名（带时间戳）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filePath = logDirPath + "/VitroSim_" + timestamp + ".log";

    // 打开文件
    m_file.setFileName(filePath);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_stream.setDevice(&m_file);
        m_isOpen = true;
        // 写入第一条日志，记录程序启动
        writeLog("INFO", "程序启动，日志系统初始化成功");
    } else {
        // 如果文件打开失败，直接在控制台输出错误（因为日志系统本身不能用）
        qWarning() << "无法创建日志文件：" << filePath;
    }
}

Logger::~Logger()
{
    if (m_isOpen) {
        writeLog("INFO", "程序正常退出，日志系统关闭");
        m_file.close();
    }
}

void Logger::writeLog(const QString &level, const QString &msg)
{
    // 如果文件未正常打开，则直接返回（避免崩溃）
    if (!m_isOpen) return;

    // 加锁保护，防止多线程同时写入
    QMutexLocker locker(&m_mutex);

    // 生成时间戳
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 写入日志行
    m_stream << "[" << timeStr << "] " << level << ": " << msg << "\n";
    m_stream.flush();   // 立即写入磁盘，防止程序崩溃时丢失日志
}