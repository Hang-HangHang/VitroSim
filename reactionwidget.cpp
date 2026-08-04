#include "reactionwidget.h"
#include <QPainter>
#include <QtMath>
#include <QMouseEvent>

ReactionWidget::ReactionWidget(QWidget *parent)
    : QWidget(parent), m_hoveredIndex(-1)
{
    setMinimumSize(300, 300);
    setMouseTracking(true);
    for (int i = 0; i < 32; ++i) {
        m_colors.push_back(Qt::gray);
        m_texts.push_back(QString::number(i + 1));
        m_selected.push_back(false);
    }
}

void ReactionWidget::setWellColor(int index, QColor color)
{
    if (index >= 0 && index < 32) {
        m_colors[index] = color;
        update();
    }
}

void ReactionWidget::setWellText(int index, const QString& text)
{
    if (index >= 0 && index < 32) {
        m_texts[index] = text;
        update();
    }
}

void ReactionWidget::setWellSelected(int index, bool selected)
{
    if (index >= 0 && index < 32) {
        m_selected[index] = selected;
        update();
    }
}

bool ReactionWidget::isWellSelected(int index) const
{
    return (index >= 0 && index < 32) ? m_selected[index] : false;
}

QList<int> ReactionWidget::getSelectedChannels() const
{
    QList<int> list;
    for (int i = 0; i < 32; ++i) {
        if (m_selected[i]) list.append(i + 1);
    }
    return list;
}

QColor ReactionWidget::getWellColor(int index) const
{
    if (index >= 0 && index < 32) return m_colors[index];
    return Qt::gray;
}

void ReactionWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int w = width(), h = height();
    int cx = w/2, cy = h/2;

    // 外圈半径：留出边距
    int margin = 20;
    int radius = qMin(w, h)/2 - margin;
    if (radius < 50) radius = 50; // 防止太小

    // 轨道半径：孔位中心到圆心的距离，留出孔位大小空间
    int orbitRadius = radius - 20; // 比外圈小一些

    // 计算孔位半径：使相邻孔位刚好不重叠，并留有余量
    // 32个孔均匀分布，每个孔占据角度 = 2*PI/32
    // 孔位半径最大为 orbitRadius * sin(PI/32) * 0.9
    double maxWellRadius = orbitRadius * sin(M_PI / 32) * 0.9;
    // 但也要考虑显示文字，至少6px
    double wellRadius = qMax(6.0, maxWellRadius);
    // 限制最大不超过20
    if (wellRadius > 20) wellRadius = 20;

    painter.setPen(Qt::black);
    painter.drawEllipse(cx - radius, cy - radius, 2*radius, 2*radius);

    // 绘制孔位
    for (int i = 0; i < 32; ++i) {
        double angle = i * (2 * M_PI / 32) - M_PI/2;
        int x = cx + orbitRadius * cos(angle) - wellRadius;
        int y = cy + orbitRadius * sin(angle) - wellRadius;

        QColor color = m_colors[i];

        if (m_selected[i]) {
            painter.setPen(QPen(Qt::yellow, 2));
        } else if (i == m_hoveredIndex) {
            painter.setPen(QPen(Qt::black, 2));
        } else {
            painter.setPen(Qt::black);
        }

        painter.setBrush(color);
        painter.drawEllipse(QPointF(cx + orbitRadius * cos(angle), cy + orbitRadius * sin(angle)),
                            wellRadius, wellRadius);

        // 文字（如果孔位足够大才显示数字，否则只显示状态符号）
        if (wellRadius > 8) {
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", qMax(6, (int)(wellRadius * 0.6))));
            QRectF textRect(cx + orbitRadius * cos(angle) - wellRadius,
                            cy + orbitRadius * sin(angle) - wellRadius,
                            wellRadius*2, wellRadius*2);
            painter.drawText(textRect, Qt::AlignCenter, m_texts[i]);
        } else {
            // 太小就不显示文字，用颜色表示
        }
    }
}

void ReactionWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();
    int found = -1;
    double minDist = 1e9;

    // 动态计算当前孔位半径（与paintEvent保持一致）
    int w = width(), h = height();
    int cx = w/2, cy = h/2;
    int margin = 20;
    int radius = qMin(w, h)/2 - margin;
    if (radius < 50) radius = 50;
    int orbitRadius = radius - 20;
    double maxWellRadius = orbitRadius * sin(M_PI / 32) * 0.9;
    double wellRadius = qMax(6.0, maxWellRadius);
    if (wellRadius > 20) wellRadius = 20;
    double hitRadius = wellRadius + 5; // 增加点击容差

    for (int i = 0; i < 32; ++i) {
        double angle = i * (2 * M_PI / 32) - M_PI/2;
        QPointF center(cx + orbitRadius * cos(angle), cy + orbitRadius * sin(angle));
        double dist = QPointF::dotProduct(pos - center, pos - center);
        if (dist < hitRadius * hitRadius) {
            if (dist < minDist) {
                minDist = dist;
                found = i;
            }
        }
    }

    if (found >= 0) {
        m_selected[found] = !m_selected[found];
        update();
        emit selectionChanged(getSelectedChannels());
        emit wellClicked(found + 1);
    }
}

void ReactionWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();
    int newHover = -1;
    double minDist = 1e9;

    int w = width(), h = height();
    int cx = w/2, cy = h/2;
    int margin = 20;
    int radius = qMin(w, h)/2 - margin;
    if (radius < 50) radius = 50;
    int orbitRadius = radius - 20;
    double maxWellRadius = orbitRadius * sin(M_PI / 32) * 0.9;
    double wellRadius = qMax(6.0, maxWellRadius);
    if (wellRadius > 20) wellRadius = 20;
    double hitRadius = wellRadius + 5;

    for (int i = 0; i < 32; ++i) {
        double angle = i * (2 * M_PI / 32) - M_PI/2;
        QPointF center(cx + orbitRadius * cos(angle), cy + orbitRadius * sin(angle));
        double dist = QPointF::dotProduct(pos - center, pos - center);
        if (dist < hitRadius * hitRadius) {
            if (dist < minDist) {
                minDist = dist;
                newHover = i;
            }
        }
    }

    if (newHover != m_hoveredIndex) {
        m_hoveredIndex = newHover;
        setCursor(m_hoveredIndex >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}

void ReactionWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_hoveredIndex = -1;
    setCursor(Qt::ArrowCursor);
    update();
}

QPointF ReactionWidget::getWellCenter(int index) const
{
    int w = width(), h = height();
    int cx = w/2, cy = h/2;
    int margin = 20;
    int radius = qMin(w, h)/2 - margin;
    if (radius < 50) radius = 50;
    int orbitRadius = radius - 20;
    double angle = index * (2 * M_PI / 32) - M_PI/2;
    return QPointF(cx + orbitRadius * cos(angle), cy + orbitRadius * sin(angle));
}