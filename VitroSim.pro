QT       += core widgets charts network printsupport sql
CONFIG   += c++17

SOURCES += \
    historydialog.cpp \
    historymanager.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    testmanager.cpp \
    reactionwidget.cpp \
    deviceserver.cpp \
    datareceiver.cpp \
    calibrationmanager.cpp

HEADERS += \
    historydialog.h \
    historymanager.h \
    logger.h \
    mainwindow.h \
    testmanager.h \
    reactionwidget.h \
    deviceserver.h \
    datareceiver.h \
    calibrationmanager.h

QMAKE_CXXFLAGS += /utf-8

RESOURCES += \
    resources.qrc