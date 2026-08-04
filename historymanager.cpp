#include "historymanager.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

HistoryManager::HistoryManager(QObject *parent)
    : QObject(parent)
{
    // 数据库文件保存在程序所在目录的 data/ 文件夹中
    QString dataDir = QCoreApplication::applicationDirPath() + "/data";
    QDir dir;
    if (!dir.exists(dataDir)) {
        dir.mkpath(dataDir);   // 如果目录不存在则创建
    }
    m_dbPath = dataDir + "/vitrosim_history.db";

    // 自动初始化数据库
    initDatabase();
}

bool HistoryManager::initDatabase()
{
    // 如果数据库已打开，先关闭
    if (m_db.isOpen()) {
        m_db.close();
    }

    // 使用 SQLite 驱动，连接名称为 "history_connection"（避免与其他连接冲突）
    m_db = QSqlDatabase::addDatabase("QSQLITE", "history_connection");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qDebug() << "数据库打开失败:" << m_db.lastError().text();
        emit operationCompleted(false, "数据库打开失败: " + m_db.lastError().text());
        return false;
    }

    // 创建表（如果不存在）
    // 字段说明：
    // id         : 自增主键
    // timestamp  : 测试时间，格式 yyyy-MM-dd HH:mm:ss
    // avg_od     : 平均吸光度，浮点数
    // concentration : 浓度，浮点数
    // point_count : 采集点数，整数
    // raw_data   : 原始OD值列表，以JSON数组格式存储
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS test_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            avg_od REAL,
            concentration REAL,
            point_count INTEGER,
            raw_data TEXT
        )
    )";

    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        qDebug() << "创建表失败:" << query.lastError().text();
        emit operationCompleted(false, "创建表失败: " + query.lastError().text());
        return false;
    }

    emit operationCompleted(true, "数据库初始化成功");
    return true;
}

bool HistoryManager::insertRecord(double avgOd, double concentration, int pointCount, const QList<double>& odList)
{
    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            emit operationCompleted(false, "数据库未打开");
            return false;
        }
    }

    // 将 QList<double> 转换为 JSON 数组字符串
    QJsonArray jsonArray;
    for (double value : odList) {
        jsonArray.append(value);
    }
    QJsonDocument jsonDoc(jsonArray);
    QString rawDataJson = jsonDoc.toJson(QJsonDocument::Compact);   // 紧凑格式，节省空间

    // 获取当前时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 构建插入语句（使用占位符防止SQL注入）
    QString insertSQL = R"(
        INSERT INTO test_records (timestamp, avg_od, concentration, point_count, raw_data)
        VALUES (?, ?, ?, ?, ?)
    )";

    QSqlQuery query(m_db);
    query.prepare(insertSQL);
    query.addBindValue(timestamp);
    query.addBindValue(avgOd);
    query.addBindValue(concentration);
    query.addBindValue(pointCount);
    query.addBindValue(rawDataJson);

    if (!query.exec()) {
        qDebug() << "插入记录失败:" << query.lastError().text();
        emit operationCompleted(false, "插入记录失败: " + query.lastError().text());
        return false;
    }

    emit operationCompleted(true, "记录已保存");
    return true;
}

QList<TestRecord> HistoryManager::getAllRecords()
{
    QList<TestRecord> records;

    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            emit operationCompleted(false, "数据库未打开");
            return records;
        }
    }

    // 按 ID 倒序查询（最新的记录在前）
    QString selectSQL = R"(
        SELECT id, timestamp, avg_od, concentration, point_count, raw_data
        FROM test_records
        ORDER BY id ASC
    )";

    QSqlQuery query(m_db);
    if (!query.exec(selectSQL)) {
        qDebug() << "查询记录失败:" << query.lastError().text();
        emit operationCompleted(false, "查询记录失败: " + query.lastError().text());
        return records;
    }

    while (query.next()) {
        TestRecord record;
        record.id = query.value("id").toInt();
        record.timestamp = query.value("timestamp").toString();
        record.avgOd = query.value("avg_od").toDouble();
        record.concentration = query.value("concentration").toDouble();
        record.pointCount = query.value("point_count").toInt();
        record.rawData = query.value("raw_data").toString();
        records.append(record);
    }

    return records;
}

bool HistoryManager::deleteRecord(int id)
{
    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            emit operationCompleted(false, "数据库未打开");
            return false;
        }
    }

    QString deleteSQL = "DELETE FROM test_records WHERE id = ?";
    QSqlQuery query(m_db);
    query.prepare(deleteSQL);
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "删除记录失败:" << query.lastError().text();
        emit operationCompleted(false, "删除记录失败: " + query.lastError().text());
        return false;
    }

    emit operationCompleted(true, "记录已删除");
    return true;
}

bool HistoryManager::clearAllRecords()
{
    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            emit operationCompleted(false, "数据库未打开");
            return false;
        }
    }

    QString clearSQL = "DELETE FROM test_records";
    QSqlQuery query(m_db);
    if (!query.exec(clearSQL)) {
        qDebug() << "清空记录失败:" << query.lastError().text();
        emit operationCompleted(false, "清空记录失败: " + query.lastError().text());
        return false;
    }

    emit operationCompleted(true, "所有记录已清空");
    return true;
}

int HistoryManager::getRecordCount()
{
    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            return 0;
        }
    }

    QString countSQL = "SELECT COUNT(*) FROM test_records";
    QSqlQuery query(m_db);
    if (!query.exec(countSQL)) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}