#ifndef REACTIONWIDGET_H
#define REACTIONWIDGET_H

#include <QWidget>
#include <QColor>
#include <vector>
#include <QList>

class ReactionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReactionWidget(QWidget *parent = nullptr);

    void setWellColor(int index, QColor color);
    void setWellText(int index, const QString& text);
    void setWellSelected(int index, bool selected);
    bool isWellSelected(int index) const;
    QList<int> getSelectedChannels() const;

    // 新增：获取孔位颜色（用于判断完成状态）
    QColor getWellColor(int index) const;

signals:
    void selectionChanged(QList<int> selectedChannels);  // 选中列表变化
    void wellClicked(int index);                         // 点击孔位（1~32）

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    std::vector<QColor> m_colors;
    std::vector<QString> m_texts;
    std::vector<bool> m_selected;
    int m_hoveredIndex;

    QPointF getWellCenter(int index) const;
};

#endif // REACTIONWIDGET_H