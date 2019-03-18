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

LIBS += C:\Qt\Qt5.12.0\Tools\mingw730_64\x86_64-w64-mingw32\lib\libws2_32.a
