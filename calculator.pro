QT += qml

CONFIG += c++17

HEADERS += interaction.h \
    calcworker.h \
    commom.h

SOURCES += interaction.cpp \
    calcworker.cpp \
    main.cpp

RESOURCES += resource.qrc

target.path = ./bin
INSTALLS += target

win32:RC_FILE = calc.rc
