#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QList>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<QList<double>>("QList<double>");

    // ---- 加载 QSS 样式表 ----
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QString::fromUtf8(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    } else {
        // 如果加载失败，输出错误信息（调试用）
        qDebug() << "样式表加载失败！";
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}