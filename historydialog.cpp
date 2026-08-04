#include "historydialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("📋 历史记录");
    setMinimumSize(650, 400);

    m_manager = new HistoryManager(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* topLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel("📋 测试历史记录");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_countLabel = new QLabel("共 0 条记录");
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_countLabel);
    mainLayout->addLayout(topLayout);

    m_tableView = new QTableView(this);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 5列：序号（自增）、时间、平均OD、浓度、点数
    m_model = new QStandardItemModel(0, 5, this);
    m_model->setHorizontalHeaderLabels({"序号", "时间", "平均OD", "浓度(mmol/L)", "采集点数"});
    m_tableView->setModel(m_model);

    // 设置列宽（序号列稍窄）
    m_tableView->setColumnWidth(0, 60);
    m_tableView->setColumnWidth(1, 160);
    m_tableView->setColumnWidth(2, 100);
    m_tableView->setColumnWidth(3, 130);
    m_tableView->setColumnWidth(4, 80);

    mainLayout->addWidget(m_tableView);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton("🔄 刷新", this);
    m_deleteBtn = new QPushButton("🗑️ 删除选中", this);
    m_clearBtn = new QPushButton("🧹 清空所有", this);
    QPushButton* closeBtn = new QPushButton("✖ 关闭", this);

    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &HistoryDialog::refreshData);
    connect(m_deleteBtn, &QPushButton::clicked, this, &HistoryDialog::onDeleteSelected);
    connect(m_clearBtn, &QPushButton::clicked, this, &HistoryDialog::onClearAll);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    refreshData();
    setWindowModality(Qt::ApplicationModal);
}

void HistoryDialog::refreshData()
{
    m_model->removeRows(0, m_model->rowCount());

    // 获取所有记录（现在按时间升序）
    QList<TestRecord> records = m_manager->getAllRecords();

    int rowNum = 1;  // 序号从1开始
    for (const TestRecord& record : records) {
        QList<QStandardItem*> rowItems;

        // 序号列（自增，不依赖ID）
        QStandardItem* numItem = new QStandardItem(QString::number(rowNum));
        numItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(numItem);

        // 时间列
        QStandardItem* timeItem = new QStandardItem(record.timestamp);
        timeItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(timeItem);

        // 平均OD列
        QStandardItem* odItem = new QStandardItem(QString::number(record.avgOd, 'f', 4));
        odItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(odItem);

        // 浓度列
        QStandardItem* concItem = new QStandardItem(QString::number(record.concentration, 'f', 2));
        concItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(concItem);

        // 点数列
        QStandardItem* countItem = new QStandardItem(QString::number(record.pointCount));
        countItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(countItem);

        // 存储真实ID到第一个项目的Data中（用于删除）
        numItem->setData(record.id, Qt::UserRole);

        m_model->appendRow(rowItems);
        rowNum++;
    }

    updateCount();
}

void HistoryDialog::updateCount()
{
    int count = m_manager->getRecordCount();
    m_countLabel->setText(QString("共 %1 条记录").arg(count));
}

void HistoryDialog::onDeleteSelected()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一条记录");
        return;
    }

    // 获取选中行的真实ID（存储在UserRole中）
    int row = selected.first().row();
    int realId = m_model->item(row, 0)->data(Qt::UserRole).toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除序号 %1 的记录吗？").arg(row + 1),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (m_manager->deleteRecord(realId)) {
            refreshData();
            QMessageBox::information(this, "成功", "记录已删除");
        } else {
            QMessageBox::warning(this, "错误", "删除失败，请查看日志");
        }
    }
}

void HistoryDialog::onClearAll()
{
    int count = m_manager->getRecordCount();
    if (count == 0) {
        QMessageBox::information(this, "提示", "没有记录可清空");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认清空",
        QString("确定要清空所有 %1 条记录吗？\n此操作不可恢复！").arg(count),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (m_manager->clearAllRecords()) {
            refreshData();
            QMessageBox::information(this, "成功", "所有记录已清空");
        } else {
            QMessageBox::warning(this, "错误", "清空失败，请查看日志");
        }
    }
}