# http://doc.qt.io/qt-5/qmake-variable-reference.html
CONFIG += console
QT += core widgets gui
DEFINES += QT_WIDGETS_LIB
INCLUDEPATH += .

HEADERS += paint.h \
           canvas.h
SOURCES += paint.cpp \
           canvas.cpp \
           main.cpp
