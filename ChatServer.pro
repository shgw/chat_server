TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    log.cpp \
    serversocket.cpp \
    socket.cpp \
    svrsock.cpp

HEADERS += \
    log.h \
    serversocket.h \
    socket.h \
    svrsock.h \
    socketmessage.h

DEFINES += __LINUX
