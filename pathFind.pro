QT       += core gui charts concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


SOURCES += \
    GridView.cpp \
    MazeGenerator.cpp \
    PathAlgorithm.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    GridView.h \
    PathAlgorithm.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
