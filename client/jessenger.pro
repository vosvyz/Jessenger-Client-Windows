QT       += core gui network websockets sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cachingmanager.cpp \
    chatpushbutton.cpp \
    httpclient.cpp \
    messagewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    networkclient.cpp \
    persistenttokenstorage.cpp \
    ramtokenstorage.cpp \
    wsclient.cpp

HEADERS += \
    cachingmanager.h \
    chatpushbutton.h \
    httpclient.h \
    messagewidget.h \
    mainwindow.h \
    networkclient.h \
    persistenttokenstorage.h \
    ramtokenstorage.h \
    wsclient.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += "../websocketpp-0.8.2/websocketpp"
DEPENDPATH += "../websocketpp-0.8.2/websocketpp"

DISTFILES += \
    TODO
