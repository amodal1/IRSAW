TARGET = JouleSAW
INCLUDEPATH += .

QT += quick bluetooth
#QT += bluetooth

# Input
SOURCES += main.cpp \
    device.cpp \
    deviceinfo.cpp \
    serviceinfo.cpp \
    characteristicinfo.cpp

OTHER_FILES += assets/*.qml

HEADERS += \
    device.h \
    deviceinfo.h \
    serviceinfo.h \
    characteristicinfo.h

RESOURCES += \
    resources.qrc

target.path = ~/Projects/bluetooth/JouleSAW
INSTALLS += target
