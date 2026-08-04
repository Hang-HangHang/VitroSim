#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include <QObject>
#include <QVector>
#include <QPointF>

/**
 * @brief 标准曲线管理器
 * 负责通过已知浓度的标准品建立浓度-吸光度拟合直线（最小二乘法），
 * 并提供根据OD值计算浓度、判断OD是否在有效范围内的功能。
 * 这是IVD设备的核心算法模块。
 */
class CalibrationManager : public QObject
{
    Q_OBJECT
public:
    explicit CalibrationManager(QObject *parent = nullptr);

    /// 添加一个标定点 (浓度, 对应的吸光度OD)
    void addStandardPoint(double concentration, double od);

    /// 执行线性拟合，返回是否成功
    bool fit();

    /// 根据给定的吸光度OD值，计算对应的浓度
    double calculateConcentration(double od) const;

    /// 获取拟合后的斜率 (k)
    double getSlope() const { return m_slope; }
    /// 获取拟合后的截距 (b)
    double getIntercept() const { return m_intercept; }
    /// 获取决定系数 R² (越接近1拟合越好)
    double getR2() const { return m_r2; }

    /// 获取所有标定点 (用于绘制标准曲线图)
    QVector<QPointF> getStandardPoints() const { return m_points; }

    /// 判断给定的OD值是否在标定范围内 (用于异常报警)
    bool isOdInRange(double od) const;

signals:
    /// 定标状态更新，如成功或失败信息
    void calibrationUpdated(QString message);

private:
    QVector<QPointF> m_points;   // 存储标定点 (x=浓度, y=OD)
    double m_slope;              // 拟合直线斜率
    double m_intercept;          // 拟合直线截距
    double m_r2;                 // 决定系数
    double m_minOD;              // 标定点中最小OD值
    double m_maxOD;              // 标定点中最大OD值
    bool m_isFitted;             // 是否已完成拟合
};

#endif // CALIBRATIONMANAGER_H