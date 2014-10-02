#-------------------------------------------------
#
# Project created by QtCreator 2013-10-27T12:27:23
#
#-------------------------------------------------
QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += .

TARGET = gsorf
TEMPLATE = app

SOURCES += main.cpp\
        dialog.cpp \
    task_client.cpp \
    blenderrenderengine.cpp \
    udpmanager.cpp \
    client.cpp \
    result.cpp \
    task_server.cpp \
    taskhandler.cpp \
    webserver.cpp \
    websocketrunnable.cpp

HEADERS  += dialog.h \
    task_client.h \
    blenderrenderengine.h \
    udpmanager.h \
    client.h \
    result.h \
    task_server.h \
    taskhandler.h \
    webserver.h \
    websocketrunnable.h

FORMS    += dialog.ui

OTHER_FILES += \
assets/cpu.jpg \
assets/gpu.jpg

#copy assets folder into build directory
linux-g++{
    QMAKE_POST_LINK += $$quote (cp -rf $${PWD}/assets $${OUT_PWD}) $$escape_expand(\n\t)
    QMAKE_POST_LINK += $$quote (cp -rf $${PWD}/htdocs $${OUT_PWD})
}

win32{
    PWD_WIN = $${PWD}
    PWD_WIN ~= s,/,\\,g
    OUT_PWD_WIN = $${OUT_PWD}
    OUT_PWD_WIN ~= s,/,\\,g
    QMAKE_POST_LINK += $$quote(xcopy $${PWD_WIN}\\assets $${OUT_PWD_WIN}\\assets /EIY) $$escape_expand(\n\t)
    QMAKE_POST_LINK += $$quote(xcopy $${PWD_WIN}\\htdocs $${OUT_PWD_WIN}\\htdocs /EIY)
}

