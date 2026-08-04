#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include "historymanager.h"

/**
 * @brief HistoryDialog 显示所有历史测试记录的对话框
 *
 * 功能：
 * - 以表格形式展示所有历史记录（ID、时间、平均OD、浓度、点数）
 * - 支持选中单条记录并删除
 * - 支持一键清空所有记录
 * - 支持手动刷新数据
 * - 显示记录总数
 */
class HistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDialog(QWidget *parent = nullptr);

    /// 刷新表格数据（重新从数据库加载）
    void refreshData();

private slots:
    void onDeleteSelected();   // 删除选中的记录
    void onClearAll();         // 清空所有记录

private:
    HistoryManager* m_manager;          // 数据库管理器
    QTableView* m_tableView;            // 表格视图
    QStandardItemModel* m_model;        // 表格数据模型（5列）
    QPushButton* m_deleteBtn;           // 删除选中按钮
    QPushButton* m_clearBtn;            // 清空所有按钮
    QPushButton* m_refreshBtn;          // 刷新按钮
    QLabel* m_countLabel;               // 记录数量标签

    /// 设置表格样式（列宽、选择模式等）
    void setupTable();

    /// 更新记录数量显示
    void updateCount();
};

#endif // HISTORYDIALOG_H