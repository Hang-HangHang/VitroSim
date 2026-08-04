#include "calibrationmanager.h"
#include <cmath>
#include <QDebug>

CalibrationManager::CalibrationManager(QObject *parent)
    : QObject(parent), m_slope(0), m_intercept(0), m_r2(0),
    m_minOD(0), m_maxOD(0), m_isFitted(false)
{
    // ---------- 模拟真实设备：预设标准品数据 ----------
    // 在实际生产中，这些数据是通过测量已知浓度的标准品获得的。
    // 此处使用一组典型数据模拟定标过程。
    addStandardPoint(0.0, 0.05);   // 零浓度空白
    addStandardPoint(5.0, 1.05);
    addStandardPoint(10.0, 2.10);
    addStandardPoint(20.0, 4.15);
    addStandardPoint(50.0, 9.80);
    // -------------------------------------------------

    if (fit()) {
        // 定标成功，发出信号供界面显示
        emit calibrationUpdated(QString("✅ 定标成功！斜率= %1, 截距= %2, R²= %3")
                                    .arg(m_slope, 0, 'f', 4)
                                    .arg(m_intercept, 0, 'f', 4)
                                    .arg(m_r2, 0, 'f', 6));
    } else {
        emit calibrationUpdated("❌ 定标失败，请检查标定点数据");
    }
}

void CalibrationManager::addStandardPoint(double concentration, double od)
{
    m_points.append(QPointF(concentration, od));
}

bool CalibrationManager::fit()
{
    // 至少需要两个点才能拟合直线
    if (m_points.size() < 2) return false;

    int n = m_points.size();
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;

    // 初始化极值
    m_minOD = m_points[0].y();
    m_maxOD = m_points[0].y();

    // 计算各类求和
    for (const QPointF &p : m_points) {
        double x = p.x(); // 浓度
        double y = p.y(); // OD
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        sumY2 += y * y;
        // 更新OD极值
        if (y < m_minOD) m_minOD = y;
        if (y > m_maxOD) m_maxOD = y;
    }

    // 分母不能为0 (所有浓度相同无法拟合)
    double denominator = (n * sumX2 - sumX * sumX);
    if (fabs(denominator) < 1e-10) return false;

    // ------- 最小二乘法拟合直线 y = k*x + b -------
    m_slope = (n * sumXY - sumX * sumY) / denominator;
    m_intercept = (sumY - m_slope * sumX) / n;

    // 计算决定系数 R²
    double meanY = sumY / n;
    double ssTot = 0;   // 总平方和
    double ssRes = 0;   // 残差平方和
    for (const QPointF &p : m_points) {
        double y = p.y();
        double y_pred = m_slope * p.x() + m_intercept; // 预测值
        ssTot += (y - meanY) * (y - meanY);
        ssRes += (y - y_pred) * (y - y_pred);
    }
    m_r2 = 1 - (ssRes / ssTot);

    m_isFitted = true;
    return true;
}

double CalibrationManager::calculateConcentration(double od) const
{
    if (!m_isFitted) return -1.0;
    // 根据拟合公式：浓度 = (OD - 截距) / 斜率
    return (od - m_intercept) / m_slope;
}

bool CalibrationManager::isOdInRange(double od) const
{
    if (!m_isFitted) return false;
    // 如果OD值低于最低点或高于最高点（留一点余量），认为异常
    return (od >= m_minOD - 0.1 && od <= m_maxOD + 0.1);
}