#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QThread>
#include <QCheckBox>
#include <QSpinBox>
#include <QTcpSocket>
#include "testmanager.h"
#include "reactionwidget.h"
#include "deviceserver.h"
#include "datareceiver.h"
#include "calibrationmanager.h"

QT_CHARTS_USE_NAMESPACE

    class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStart();
    void onPause();
    void onReset();
    void updateState(State s);
    void onExportCsv();
    void onExportPdf();
    void onShowHistory();
    void onSmoothToggled(bool checked);
    void onFilterWindowChanged(int value);
    void onSendLis();
    void onLisConnected();
    void onLisDisconnected();
    void onLisError(QAbstractSocket::SocketError error);
    void onLisSent();

private:
    // 核心组件
    TestManager      *m_manager;
    ReactionWidget   *m_wheel;
    QChartView       *m_chartView;
    QLineSeries      *m_series;
    QLineSeries      *m_smoothSeries;
    QPushButton      *m_startBtn, *m_pauseBtn, *m_resetBtn;
    QLabel           *m_statusLabel;
    QLabel           *m_resultLabel;
    int               m_counter;
    int               m_smoothCounter;

    // 导出按钮
    QPushButton      *m_exportCsvBtn;
    QPushButton      *m_exportPdfBtn;
    QPushButton      *m_historyBtn;

    // 平滑滤波控件
    QCheckBox        *m_smoothCheckBox;
    QSpinBox         *m_filterSpinBox;

    // LIS 对接控件
    QPushButton      *m_lisSendBtn;
    QLabel           *m_lisStatusLabel;

    // 网络与通信
    DeviceServer     *m_deviceServer;
    QThread          *m_receiverThread;
    DataReceiver     *m_receiver;

    // 算法引擎
    CalibrationManager *m_calibration;

    // 平滑滤波数据
    QList<double>    m_rawBuffer;
    int              m_filterWindowSize;

    // LIS 相关
    QTcpSocket*      m_lisSocket;
    bool             m_lisConnected;
    QString          m_lastTimestamp;
    QString          m_lastAvgOd;
    QString          m_lastConcentration;

    // 当前显示通道
    int              m_currentChannel;

    // 辅助函数
    double calculateSmoothValue(const QList<double>& buffer);
    void sendToLis(const QString& timestamp, double avgOd, double concentration);
    void updateLisStatus(const QString& status, bool isError = false);
    QString generateLisMessage(const QString& timestamp, double avgOd, double concentration);
    void updateChartForChannel(int channel);

    // 🔥 新增：更新指定通道的结果标签
    void updateResultForChannel(int channel);
};

#endif // MAINWINDOW_H