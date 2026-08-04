#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDateTime>
#include <QVariant>

/**
 * @brief 测试记录数据结构体
 */
struct TestRecord {
    int id;                 // 记录ID（自增主键）
    QString timestamp;      // 测试时间
    double avgOd;           // 平均吸光度
    double concentration;   // 检测浓度
    int pointCount;         // 采集点数
    QString rawData;        // 原始OD数据（JSON格式字符串，存储所有OD值）
};

/**
 * @brief HistoryManager 类负责管理 SQLite 数据库中的测试历史记录
 *
 * 核心功能：
 * - 初始化数据库（自动创建表）
 * - 插入新的测试记录
 * - 查询所有历史记录（按时间倒序）
 * - 根据ID删除单条记录
 * - 清空所有记录
 * - 获取记录总数
 */
class HistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit HistoryManager(QObject *parent = nullptr);

    /// 初始化数据库，创建 test_records 表（如果不存在）
    bool initDatabase();

    /// 插入一条测试记录（参数：平均OD、浓度、点数、OD值列表）
    bool insertRecord(double avgOd, double concentration, int pointCount, const QList<double>& odList);

    /// 获取所有历史记录（按ID倒序，即最新的在前）
    QList<TestRecord> getAllRecords();

    /// 根据记录ID删除一条记录
    bool deleteRecord(int id);

    /// 清空所有记录（慎用）
    bool clearAllRecords();

    /// 获取数据库中记录总数
    int getRecordCount();

    /// 获取数据库文件路径（用于调试）
    QString getDatabasePath() const { return m_dbPath; }

signals:
    /// 操作完成信号，用于通知界面操作结果
    void operationCompleted(bool success, QString message);

private:
    QSqlDatabase m_db;      // 数据库连接对象
    QString m_dbPath;       // 数据库文件完整路径

    /// 执行SQL语句的辅助方法（用于简单增删改）
    bool executeQuery(const QString& sql, const QVariantList& params = QVariantList());
};

#endif // HISTORYMANAGER_H