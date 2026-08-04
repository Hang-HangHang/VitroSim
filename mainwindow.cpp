#include "mainwindow.h"
#include <QStatusBar>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QPdfWriter>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include <QApplication>
#include <QChart>
#include <QFontMetrics>
#include "logger.h"
#include "historymanager.h"
#include "historydialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_counter(0)
    , m_smoothCounter(0)
    , m_filterWindowSize(5)
    , m_lisConnected(false)
    , m_currentChannel(1)
{
    setWindowTitle("VitroSim - 虚拟生化分析仪 (高级版)");
    Logger::info("主窗口初始化完成");

    // ==================== 1. TCP 控制服务器 ====================
    m_deviceServer = new DeviceServer(this);
    m_deviceServer->startServer(8888);
    Logger::info("TCP控制服务器已启动，端口8888");

    connect(m_deviceServer, &DeviceServer::commandReceived, this, [this](QString cmd){
        if (cmd == "START") {
            onStart();
        } else if (cmd == "STOP") {
            onPause();
        } else if (cmd == "RESET") {
            onReset();
        } else {
            statusBar()->showMessage("收到未知指令: " + cmd);
        }
    });

    resize(900, 650);

    // ==================== 2. 核心控件 ====================
    m_manager = new TestManager(this);
    m_wheel = new ReactionWidget(this);

    connect(m_wheel, &ReactionWidget::selectionChanged, this, [this](QList<int> selected){
        State state = m_manager->getState();
        if (state == Idle || state == Preheating) {
            m_manager->setActiveChannels(selected);
            if (selected.isEmpty()) {
                statusBar()->showMessage("未选中任何通道", 2000);
            } else {
                QStringList chs;
                for (int ch : selected) chs << QString::number(ch);
                statusBar()->showMessage("已选中通道: " + chs.join(", "), 2000);
            }
        } else {
            QList<int> original = m_manager->getActiveChannels();
            for (int i = 1; i <= 32; ++i) {
                m_wheel->setWellSelected(i-1, original.contains(i));
            }
            statusBar()->showMessage(state == Complete ? "测试已完成，不可修改选中" : "测试进行中，不可修改选中", 2000);
        }
    });

    connect(m_wheel, &ReactionWidget::wellClicked, this, [this](int index){
        State state = m_manager->getState();
        bool testComplete = m_manager->isTestCompleted();

        if (testComplete) {
            int pointCount = m_manager->getPointCount(index);
            int maxPoints = m_manager->getMaxPoints();

            if (pointCount >= maxPoints) {
                m_currentChannel = index;
                m_manager->setActiveChannel(index);
                statusBar()->showMessage(QString("查看通道 %1 数据").arg(index), 2000);
                updateChartForChannel(index);
                updateResultForChannel(index);
            } else if (pointCount > 0 && pointCount < maxPoints) {
                statusBar()->showMessage(QString("通道 %1 采集未完成（%2/%3点）").arg(index).arg(pointCount).arg(maxPoints), 3000);
            } else {
                statusBar()->showMessage(QString("通道 %1 未进行测试").arg(index), 3000);
                m_series->clear();
                m_smoothSeries->clear();
                m_counter = 0;
                m_smoothCounter = 0;
                m_chartView->chart()->axes().first()->setRange(0, 20);
                m_resultLabel->setText(QString("通道 %1 未进行测试").arg(index));
            }
        } else if (state == Idle || state == Preheating) {
            // 空闲状态，由 selectionChanged 处理
        } else {
            statusBar()->showMessage("测试进行中，请等待完成", 2000);
        }
    });

    // ==================== 3. 实时曲线图表 ====================
    m_series = new QLineSeries();
    m_series->setName("原始数据");
    m_series->setColor(Qt::blue);

    m_smoothSeries = new QLineSeries();
    m_smoothSeries->setName("平滑数据");
    m_smoothSeries->setColor(Qt::red);

    QChart *chart = new QChart();
    chart->addSeries(m_series);
    chart->addSeries(m_smoothSeries);
    chart->createDefaultAxes();
    chart->axes().first()->setRange(0, 20);
    chart->axes().last()->setRange(0, 2.5);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(180);

    // ==================== 4. 右侧控制面板 ====================
    QWidget *panel = new QWidget;
    QVBoxLayout *pLayout = new QVBoxLayout;

    m_startBtn = new QPushButton("▶ 开始测试");
    m_startBtn->setObjectName("startBtn");
    m_pauseBtn = new QPushButton("⏸ 紧急暂停");
    m_pauseBtn->setObjectName("pauseBtn");
    m_resetBtn = new QPushButton("↺ 复位");
    m_resetBtn->setObjectName("resetBtn");
    m_statusLabel = new QLabel("状态: 空闲");
    m_statusLabel->setObjectName("statusLabel");

    m_resultLabel = new QLabel("等待测试...");
    m_resultLabel->setObjectName("resultLabel");
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setAlignment(Qt::AlignCenter);

    m_exportCsvBtn = new QPushButton("📊 导出CSV");
    m_exportCsvBtn->setEnabled(false);

    m_exportPdfBtn = new QPushButton("📄 导出PDF报告");
    m_exportPdfBtn->setEnabled(false);

    m_historyBtn = new QPushButton("📋 历史记录");
    m_historyBtn->setEnabled(true);

    // 平滑滤波控件
    QHBoxLayout *filterLayout = new QHBoxLayout();
    m_smoothCheckBox = new QCheckBox("启用平滑");
    m_smoothCheckBox->setChecked(true);
    m_filterSpinBox = new QSpinBox();
    m_filterSpinBox->setRange(3, 15);
    m_filterSpinBox->setValue(5);
    m_filterSpinBox->setPrefix("窗口: ");
    m_filterSpinBox->setSuffix(" 点");
    filterLayout->addWidget(m_smoothCheckBox);
    filterLayout->addWidget(m_filterSpinBox);
    filterLayout->addStretch();

    // LIS 对接控件
    QLabel *lisLabel = new QLabel("📡 LIS 对接");
    lisLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");

    QHBoxLayout *lisLayout = new QHBoxLayout();
    m_lisSendBtn = new QPushButton("📤 发送到LIS");
    m_lisSendBtn->setEnabled(false);
    m_lisStatusLabel = new QLabel("⚪ 未连接");
    m_lisStatusLabel->setStyleSheet("font-size: 10px;");
    lisLayout->addWidget(m_lisSendBtn);
    lisLayout->addWidget(m_lisStatusLabel);
    lisLayout->addStretch();

    pLayout->addWidget(m_startBtn);
    pLayout->addWidget(m_pauseBtn);
    pLayout->addWidget(m_resetBtn);
    pLayout->addWidget(m_statusLabel);
    pLayout->addWidget(m_resultLabel);
    pLayout->addWidget(m_exportCsvBtn);
    pLayout->addWidget(m_exportPdfBtn);
    pLayout->addWidget(m_historyBtn);
    pLayout->addLayout(filterLayout);
    pLayout->addWidget(lisLabel);
    pLayout->addLayout(lisLayout);
    pLayout->addStretch();

    panel->setLayout(pLayout);
    panel->setMaximumWidth(240);

    // ==================== 5. 主布局 ====================
    QWidget *central = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *leftLayout = new QVBoxLayout;

    leftLayout->addWidget(m_wheel, 2);
    leftLayout->addWidget(m_chartView, 1);

    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addWidget(panel, 1);

    central->setLayout(mainLayout);
    setCentralWidget(central);

    // ==================== 6. 信号槽连接 ====================
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPause);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(m_manager, &TestManager::stateChanged, this, &MainWindow::updateState);
    connect(m_manager, &TestManager::logMessage, this, [this](QString msg){
        statusBar()->showMessage(msg, 3000);
    });
    connect(m_manager, &TestManager::globalProgress, this, [this](int current, int total){
        statusBar()->showMessage(QString("采集进度: %1 / %2").arg(current).arg(total), 1000);
    });
    connect(m_manager, &TestManager::channelStateChanged, this, [this](int channel, State state){
        int idx = channel - 1;
        if (state == Complete) {
            m_wheel->setWellColor(idx, Qt::green);
            m_wheel->setWellText(idx, "✓");
        } else if (state == Testing) {
            m_wheel->setWellColor(idx, Qt::yellow);
        } else if (state == Idle) {
            m_wheel->setWellColor(idx, Qt::gray);
            m_wheel->setWellText(idx, QString::number(channel));
        }
    });

    connect(m_exportCsvBtn, &QPushButton::clicked, this, &MainWindow::onExportCsv);
    connect(m_exportPdfBtn, &QPushButton::clicked, this, &MainWindow::onExportPdf);
    connect(m_historyBtn, &QPushButton::clicked, this, &MainWindow::onShowHistory);

    connect(m_smoothCheckBox, &QCheckBox::toggled, this, &MainWindow::onSmoothToggled);
    connect(m_filterSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onFilterWindowChanged);

    connect(m_lisSendBtn, &QPushButton::clicked, this, &MainWindow::onSendLis);

    // ==================== 7. 标准曲线引擎 ====================
    m_calibration = new CalibrationManager(this);
    connect(m_calibration, &CalibrationManager::calibrationUpdated, this, [this](QString msg){
        statusBar()->showMessage(msg, 5000);
        Logger::info("定标结果: " + msg);
    });

    // ==================== 8. 数据接收子线程 ====================
    m_receiverThread = new QThread(this);
    m_receiver = new DataReceiver();
    m_receiver->moveToThread(m_receiverThread);

    connect(m_receiverThread, &QThread::started, m_receiver, [=]() {
        m_receiver->connectToDevice("127.0.0.1", 9999);
    });

    connect(m_receiver, &DataReceiver::multiOdReceived, this, [this](QList<double> values){
        m_manager->addFrame(values);
        for (int ch : m_manager->getActiveChannels()) {
            if (m_manager->getState() == Testing) {
                int count = m_manager->getPointCount(ch);
                if (count > 0 && count < m_manager->getMaxPoints()) {
                    m_wheel->setWellText(ch-1, QString::number(count));
                }
            }
        }
    });

    connect(m_receiver, &DataReceiver::connectionStatus, this, [this](QString msg){
        statusBar()->showMessage(msg, 3000);
        Logger::info(msg);
    });

    m_receiverThread->start();

    // ==================== 9. LIS 对接 ====================
    m_lisSocket = new QTcpSocket(this);
    connect(m_lisSocket, &QTcpSocket::connected, this, &MainWindow::onLisConnected);
    connect(m_lisSocket, &QTcpSocket::disconnected, this, &MainWindow::onLisDisconnected);
    connect(m_lisSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &MainWindow::onLisError);
    connect(m_lisSocket, &QTcpSocket::bytesWritten, this, &MainWindow::onLisSent);

    m_lisSocket->connectToHost("127.0.0.1", 9997);
    Logger::info("正在连接LIS服务器 127.0.0.1:9997...");
    updateLisStatus("连接中...");

    statusBar()->showMessage("就绪，等待启动");
    updateChartForChannel(1);
}

// ==================== 槽函数实现 ====================

void MainWindow::onStart()
{
    Logger::info("用户点击“开始测试”");
    if (!m_receiver->isConnected()) {
        Logger::warning("点击开始测试但未连接虚拟光度计");
        QMessageBox::warning(this, "连接错误", "未连接到虚拟光度计！\n请确保 Python 脚本已运行。");
        return;
    }
    QList<int> selected = m_wheel->getSelectedChannels();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在反应盘上选择至少一个通道（点击孔位切换选中状态）。");
        return;
    }
    m_manager->start(selected);
}

void MainWindow::onPause()
{
    Logger::warning("用户点击“紧急暂停”");
    m_manager->pause();
}

void MainWindow::onReset()
{
    Logger::info("用户点击“复位”");
    m_manager->resetAll();

    m_resultLabel->setText("等待测试...");
    m_exportCsvBtn->setEnabled(false);
    m_exportPdfBtn->setEnabled(false);
    m_lisSendBtn->setEnabled(false);

    m_series->clear();
    m_smoothSeries->clear();
    m_counter = 0;
    m_smoothCounter = 0;
    m_rawBuffer.clear();
    m_chartView->chart()->axes().first()->setRange(0, 20);

    for (int i = 1; i <= 32; ++i) {
        m_wheel->setWellColor(i - 1, Qt::gray);
        m_wheel->setWellText(i - 1, QString::number(i));
        m_wheel->setWellSelected(i - 1, false);
    }
}

void MainWindow::updateState(State s)
{
    QString text;
    switch(s) {
    case Idle:         text = "空闲"; break;
    case Preheating:   text = "预热中"; break;
    case Testing:      text = "采集中"; break;
    case Calculating:  text = "计算浓度"; break;
    case Complete:     text = "完成"; break;
    case Error:        text = "错误暂停"; break;
    }
    m_statusLabel->setText("状态: " + text);

    if (s == Complete) {
        int channel = m_manager->getActiveChannels().isEmpty() ? 1 : m_manager->getActiveChannels().first();
        updateResultForChannel(channel);

        for (int ch : m_manager->getActiveChannels()) {
            double avgOD = m_manager->getAvgOd(ch);
            double concentration = m_calibration->calculateConcentration(avgOD);
            if (avgOD > 0 && concentration >= 0) {
                QList<double> odList = m_manager->getOdList(ch);
                HistoryManager historyMgr;
                if (historyMgr.initDatabase()) {
                    if (historyMgr.insertRecord(avgOD, concentration, odList.size(), odList)) {
                        Logger::info(QString("通道 %1 历史记录已保存").arg(ch));
                    } else {
                        Logger::warning(QString("通道 %1 历史记录保存失败").arg(ch));
                    }
                }
            }
        }

        double avgOD = m_manager->getAvgOd(channel);
        double concentration = m_calibration->calculateConcentration(avgOD);
        if (avgOD > 0 && concentration >= 0) {
            m_lastTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            m_lastAvgOd = QString::number(avgOD, 'f', 4);
            m_lastConcentration = QString::number(concentration, 'f', 2);
            m_lisSendBtn->setEnabled(true);
            m_exportCsvBtn->setEnabled(true);
            m_exportPdfBtn->setEnabled(true);
            if (m_lisConnected) {
                sendToLis(m_lastTimestamp, avgOD, concentration);
            } else {
                updateLisStatus("未连接，等待手动发送", true);
                Logger::warning("LIS未连接，结果暂未发送");
            }
        }
    }
}

void MainWindow::updateResultForChannel(int channel)
{
    double avgOD = m_manager->getAvgOd(channel);
    double concentration = m_calibration->calculateConcentration(avgOD);
    if (avgOD > 0 && concentration >= 0) {
        QString resultText = QString("🎉 检测完成！\n通道 %1 平均OD: %2\n浓度: %3 mmol/L")
                                 .arg(channel)
                                 .arg(avgOD, 0, 'f', 4)
                                 .arg(concentration, 0, 'f', 2);
        m_resultLabel->setText(resultText);
    } else if (avgOD > 0) {
        m_resultLabel->setText(QString("⚠️ 通道 %1 数据无效（浓度计算失败）").arg(channel));
    } else {
        m_resultLabel->setText(QString("通道 %1 无有效数据").arg(channel));
    }
}

void MainWindow::updateChartForChannel(int channel)
{
    QList<double> odList = m_manager->getOdList(channel);

    m_series->clear();
    m_smoothSeries->clear();
    m_counter = 0;
    m_smoothCounter = 0;

    if (odList.isEmpty()) return;

    for (double v : odList) {
        m_counter++;
        m_series->append(m_counter, v);
    }

    if (m_smoothCheckBox->isChecked()) {
        m_rawBuffer.clear();
        for (double v : odList) {
            m_rawBuffer.append(v);
            while (m_rawBuffer.size() > m_filterWindowSize) {
                m_rawBuffer.removeFirst();
            }
            double smoothValue = 0.0;
            if (m_rawBuffer.size() >= m_filterWindowSize) {
                smoothValue = calculateSmoothValue(m_rawBuffer);
            } else {
                smoothValue = v;
            }
            m_smoothCounter++;
            m_smoothSeries->append(m_smoothCounter, smoothValue);
        }
    }

    int pointCount = odList.size();
    int minX = qMax(0, pointCount - 50);
    int maxX = pointCount + 2;
    m_chartView->chart()->axes().first()->setRange(minX, maxX);
}

// ==================== 导出 CSV ====================

void MainWindow::onExportCsv()
{
    Logger::info("用户点击导出CSV");
    int channel = m_currentChannel;
    double avgOD = m_manager->getAvgOd(channel);
    double concentration = m_calibration->calculateConcentration(avgOD);

    if (concentration < 0 || avgOD <= 0) {
        Logger::warning("导出CSV失败：没有有效数据");
        QMessageBox::warning(this, "导出失败", "没有有效数据可导出");
        return;
    }

    QList<double> odList = m_manager->getOdList(channel);

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存CSV数据",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/test_data_ch" + QString::number(channel) + ".csv",
        "CSV文件 (*.csv)"
        );

    if (fileName.isEmpty()) {
        Logger::info("用户取消CSV导出");
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::error("CSV文件创建失败");
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }

    // 使用 QByteArray 直接写入 UTF-8 BOM 和数据，避免 QTextStream 编码问题
    QByteArray data;
    data.append("\xEF\xBB\xBF"); // UTF-8 BOM
    data.append("序号,吸光度(OD)\n");

    for (int i = 0; i < odList.size(); ++i) {
        data.append(QByteArray::number(i + 1));
        data.append(",");
        data.append(QByteArray::number(odList[i], 'f', 6));
        data.append("\n");
    }

    data.append("\n平均OD,");
    data.append(QByteArray::number(avgOD, 'f', 6));
    data.append("\n浓度(mmol/L),");
    data.append(QByteArray::number(concentration, 'f', 3));
    data.append("\n");

    if (file.write(data) == -1) {
        Logger::error("CSV文件写入失败");
        QMessageBox::warning(this, "错误", "写入文件失败");
        file.close();
        return;
    }

    file.close();
    Logger::info(QString("CSV已导出到: %1").arg(fileName));
    QMessageBox::information(this, "导出成功", "CSV数据已保存到:\n" + fileName);
}

// ==================== 导出 PDF（排版修复版） ====================

void MainWindow::onExportPdf()
{
    Logger::info("用户点击导出PDF报告");

    int channel = m_currentChannel;
    double avgOD = m_manager->getAvgOd(channel);
    double concentration = m_calibration->calculateConcentration(avgOD);
    if (concentration < 0 || avgOD <= 0) {
        Logger::warning("导出PDF失败：没有有效数据");
        QMessageBox::warning(this, "导出失败", "没有有效数据可导出");
        return;
    }

    QList<double> odList = m_manager->getOdList(channel);

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存PDF报告",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Report_Ch" + QString::number(channel) + "_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".pdf",
        "PDF文件 (*.pdf)"
        );
    if (fileName.isEmpty()) {
        Logger::info("用户取消PDF导出");
        return;
    }

    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(40, 40, 40, 40));
    pdfWriter.setResolution(300);

    QPainter painter(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    if (!painter.isActive()) {
        Logger::error("PDF绘制失败，无法创建画布");
        QMessageBox::warning(this, "错误", "无法生成PDF报告");
        return;
    }

    // ---------- 字体定义 ----------
    QFont titleFont("Microsoft YaHei", 20, QFont::Bold);
    QFont infoFont("Microsoft YaHei", 12);
    QFont tableHeadFont("Microsoft YaHei", 11, QFont::Bold);
    QFont tableContentFont("Microsoft YaHei", 10);
    QFont footerFont("Microsoft YaHei", 9);

    const int pageW = pdfWriter.width();
    const int pageH = pdfWriter.height();
    const int marginL = 40;
    const int marginR = 40;
    const int marginT = 40;
    const int marginB = 40;
    int y = marginT;

    // ---------- 1. 主标题（高度再次加大） ----------
    painter.setFont(titleFont);
    QRect titleRect(marginL, y, pageW - marginL - marginR, 90);   // 75→90
    painter.drawText(titleRect, Qt::AlignCenter | Qt::AlignVCenter,
                     QString("VitroSim 检测报告（通道 %1）").arg(channel));
    y += titleRect.height() + 40;   // 35→40

    // ---------- 2. 检测信息 ----------
    painter.setFont(infoFont);
    QStringList infoLines{
        QString("报告生成时间：%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")),
        QString("通道编号：%1").arg(channel),
        QString("平均吸光度（OD）：%1").arg(QString::number(avgOD, 'f', 4)),
        QString("检测浓度：%1 mmol/L").arg(QString::number(concentration, 'f', 2)),
        QString("采集点数：%1 个").arg(odList.size())
    };
    const int infoLineH = 56;          // 52→56
    const int infoLineSpacing = 60;    // 56→60
    for (const auto& line : infoLines)
    {
        QRect textRect(marginL + 10, y, pageW - marginL - marginR - 20, infoLineH);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, line);
        y += infoLineSpacing;
    }
    y += 25;

    // ---------- 3. 曲线图 ----------
    m_chartView->repaint();
    QApplication::processEvents();
    QPixmap chartPix = m_chartView->grab();
    if (!chartPix.isNull())
    {
        // 让曲线图尽可能大，宽度占满页面
        int chartMaxW = pageW - 20;       // 边距只有 10px 的留白
        int chartMaxH = 420;              // 高度加大到 420
        QPixmap scaledChart = chartPix.scaled(chartMaxW, chartMaxH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int chartX = (pageW - scaledChart.width()) / 2;
        painter.drawPixmap(chartX, y, scaledChart);
        y += scaledChart.height() + 20;
    }
    else
    {
        QRect errRect(marginL, y, pageW - marginL - marginR, 180);
        painter.drawText(errRect, Qt::AlignCenter, "曲线图加载失败");
        y += 220;
    }

    // ---------- 4. 分页判断 ----------
    if (y > pageH - 280)   // 260→280
    {
        pdfWriter.newPage();
        y = marginT;
        painter.setFont(infoFont);
        painter.drawText(QRect(marginL, y, pageW - marginL - marginR, 40),
                         Qt::AlignHCenter, QString("VitroSim 检测报告（通道 %1）- 续页").arg(channel));
        y += 50;
    }

    // ---------- 5. 数据列表标题 ----------
    painter.setFont(infoFont);
    painter.drawText(QRect(marginL, y, pageW - marginL - marginR, 60), Qt::AlignLeft, "详细数据列表");
    y += 70;

    // ---------- 6. 表格 ----------
    const int colIdxW = 150;         // 85→100，序号列继续加宽
    const int colOdW = pageW - marginL - marginR - colIdxW - 2;
    const int rowH = 52;             // 44→52，行高进一步增加
    const int cellPadding = 8;
    const QColor headBg(200, 200, 200);
    const QColor rowOddBg(245, 245, 245);
    const QColor rowEvenBg(Qt::white);

    int curRow = 0;
    int total = odList.size();

    while (curRow < total)
    {
        if (curRow > 0)
        {
            pdfWriter.newPage();
            y = marginT;
            painter.setFont(infoFont);
            painter.drawText(QRect(marginL, y, pageW - marginL - marginR, 40),
                             Qt::AlignHCenter, QString("VitroSim 检测报告（通道 %1）- 续页").arg(channel));
            y += 50;
        }

        int tableTop = y;
        painter.setFont(tableHeadFont);

        // 表头
        painter.fillRect(marginL, tableTop, colIdxW, rowH, headBg);
        painter.fillRect(marginL + colIdxW, tableTop, colOdW, rowH, headBg);
        painter.drawRect(marginL, tableTop, colIdxW, rowH);
        painter.drawRect(marginL + colIdxW, tableTop, colOdW, rowH);

        QRect headIdxRect(marginL + cellPadding, tableTop, colIdxW - cellPadding*2, rowH);
        painter.drawText(headIdxRect, Qt::AlignCenter | Qt::AlignVCenter, "序号");
        QRect headOdRect(marginL + colIdxW + cellPadding, tableTop, colOdW - cellPadding*2, rowH);
        painter.drawText(headOdRect, Qt::AlignCenter | Qt::AlignVCenter, "吸光度 (OD)");

        int contentY = tableTop + rowH;
        painter.setFont(tableContentFont);

        int usableH = pageH - contentY - marginB - 60;
        int pageMaxRow = qMax(1, usableH / rowH);
        int drawCnt = qMin(pageMaxRow, total - curRow);

        // 数据行
        for (int i = 0; i < drawCnt; ++i)
        {
            int idx = curRow + i;
            int rY = contentY + i * rowH;
            QColor bg = (i % 2 == 0) ? rowOddBg : rowEvenBg;

            painter.fillRect(marginL, rY, colIdxW, rowH, bg);
            painter.fillRect(marginL + colIdxW, rY, colOdW, rowH, bg);
            painter.drawRect(marginL, rY, colIdxW, rowH);
            painter.drawRect(marginL + colIdxW, rY, colOdW, rowH);

            QRect idxRect(marginL + cellPadding, rY, colIdxW - cellPadding*2, rowH);
            painter.drawText(idxRect, Qt::AlignCenter | Qt::AlignVCenter, QString::number(idx + 1));
            QRect odRect(marginL + colIdxW + cellPadding, rY, colOdW - cellPadding*2, rowH);
            painter.drawText(odRect, Qt::AlignCenter | Qt::AlignVCenter, QString::number(odList[idx], 'f', 6));
        }

        curRow += drawCnt;
        y = contentY + drawCnt * rowH + 15;
    }

    // ---------- 7. 页脚 ----------
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawLine(marginL, pageH - marginB, pageW - marginR, pageH - marginB);
    painter.setPen(Qt::black);
    painter.setFont(footerFont);
    QString footer = QString("生成于 VitroSim v1.0   %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    painter.drawText(QRect(marginL, pageH - marginB + 6, pageW - marginL - marginR, 25),
                     Qt::AlignRight, footer);

    painter.end();
    Logger::info(QString("PDF报告已生成: %1").arg(fileName));
    QMessageBox::information(this, "导出成功", "PDF报告已保存至：\n" + fileName);
}

// ==================== 其他功能保持不变 ====================

void MainWindow::onShowHistory()
{
    HistoryDialog dialog(this);
    dialog.exec();
}

void MainWindow::onSmoothToggled(bool checked)
{
    m_smoothSeries->setVisible(checked);
    if (!checked) {
        m_smoothSeries->clear();
        m_smoothCounter = 0;
        m_rawBuffer.clear();
    }
    Logger::info(QString("平滑滤波 %1").arg(checked ? "启用" : "禁用"));
    updateChartForChannel(m_currentChannel);
}

void MainWindow::onFilterWindowChanged(int value)
{
    m_filterWindowSize = value;
    m_rawBuffer.clear();
    m_smoothSeries->clear();
    m_smoothCounter = 0;
    Logger::info(QString("滤波窗口大小调整为 %1").arg(value));
    updateChartForChannel(m_currentChannel);
}

double MainWindow::calculateSmoothValue(const QList<double>& buffer)
{
    if (buffer.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : buffer) sum += v;
    return sum / buffer.size();
}

// ==================== LIS 对接 ====================

void MainWindow::onSendLis()
{
    if (m_lastTimestamp.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有可发送的结果数据");
        return;
    }
    if (!m_lisConnected) {
        updateLisStatus("正在重新连接...");
        m_lisSocket->connectToHost("127.0.0.1", 9997);
        return;
    }
    double avgOd = m_lastAvgOd.toDouble();
    double concentration = m_lastConcentration.toDouble();
    sendToLis(m_lastTimestamp, avgOd, concentration);
}

void MainWindow::sendToLis(const QString& timestamp, double avgOd, double concentration)
{
    if (!m_lisConnected) {
        updateLisStatus("未连接，发送失败", true);
        return;
    }
    QString message = generateLisMessage(timestamp, avgOd, concentration);
    QByteArray data = message.toUtf8();
    m_lisSocket->write(data);
    m_lisSocket->flush();
    updateLisStatus("发送中...");
    Logger::info("向LIS发送结果: " + message);
}

void MainWindow::onLisConnected()
{
    m_lisConnected = true;
    updateLisStatus("已连接 ✅");
    Logger::info("LIS服务器已连接");
    if (!m_lastTimestamp.isEmpty() && m_lisSendBtn->isEnabled()) {
        double avgOd = m_lastAvgOd.toDouble();
        double concentration = m_lastConcentration.toDouble();
        sendToLis(m_lastTimestamp, avgOd, concentration);
    }
}

void MainWindow::onLisDisconnected()
{
    m_lisConnected = false;
    updateLisStatus("已断开 ❌", true);
    Logger::warning("LIS服务器已断开");
}

void MainWindow::onLisError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    m_lisConnected = false;
    updateLisStatus("连接错误 ❌", true);
    Logger::error(QString("LIS错误: %1").arg(m_lisSocket->errorString()));
}

void MainWindow::onLisSent()
{
    updateLisStatus("发送成功 ✅");
    Logger::info("LIS数据发送成功");
}

void MainWindow::updateLisStatus(const QString& status, bool isError)
{
    m_lisStatusLabel->setText(status);
    if (isError) {
        m_lisStatusLabel->setStyleSheet("font-size: 10px; color: red;");
    } else if (status.contains("成功") || status.contains("已连接")) {
        m_lisStatusLabel->setStyleSheet("font-size: 10px; color: green;");
    } else {
        m_lisStatusLabel->setStyleSheet("font-size: 10px; color: orange;");
    }
}

QString MainWindow::generateLisMessage(const QString& timestamp, double avgOd, double concentration)
{
    QString ts = timestamp;
    ts.remove(":");
    ts.remove("-");
    ts.remove(" ");

    QString msgId = QDateTime::currentDateTime().toString("hhmmss");
    QString accessionNum = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

    QString message = QString(
                          "MSH|^~\\&|VitroSim|Device001|LIS|LIS001|%1||ORU^R01|MSG%2|P|2.3\r\n"
                          "PID|1||P001||Test^Patient\r\n"
                          "OBR|1|%3|%4|OD^吸光度^L|||%5\r\n"
                          "OBX|1|NM|OD^吸光度||%6|OD|0.0-2.5|N|||F\r\n"
                          "OBX|2|NM|CONC^浓度||%7|mmol/L|0.0-20.0|N|||F\r\n"
                          ).arg(ts)
                          .arg(msgId)
                          .arg(timestamp)
                          .arg(accessionNum)
                          .arg(timestamp)
                          .arg(avgOd, 0, 'f', 4)
                          .arg(concentration, 0, 'f', 2);

    return message;
}

// ==================== 析构函数 ====================

MainWindow::~MainWindow()
{
    if (m_lisSocket) {
        m_lisSocket->disconnectFromHost();
        m_lisSocket->deleteLater();
    }
    if (m_receiverThread) {
        m_receiverThread->quit();
        m_receiverThread->wait();
    }
}