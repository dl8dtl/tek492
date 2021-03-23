#-------------------------------------------------
#
# Project created by QtCreator 2016-06-13T11:06:21
#
#-------------------------------------------------

QT       += core gui xml printsupport

CONFIG   += qwt
INCLUDEPATH += /usr/include/qwt
#INCLUDEPATH += /usr/local/include
#INCLUDEPATH += C:\Qwt-6.1.3\include
#freebsd: LIBS += -lqwt6
linux: LIBS += -lqwt-qt5
#win32: LIBS += -LC:\Qwt-6.1.3\lib
#win32: CONFIG += static
#linux|win32|win64: LIBS += -lqwt

contains(QWT_CONFIG, QwtOpenGL ) {

    QT += opengl
}
else {

    DEFINES += QWT_NO_OPENGL
}

contains(QWT_CONFIG, QwtSvg) {

    QT += svg
}
else {

    DEFINES += QWT_NO_SVG
}


#win32 {
#    contains(QWT_CONFIG, QwtDll) {
#        DEFINES    += QT_DLL QWT_DLL
#    }
#}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tek492
TEMPLATE = app

win32|win64: INCLUDEPATH += C:\libusb-win32-bin-1.2.6.0\libusb-win32-bin-1.2.6.0\include


SOURCES += main.cpp\
        tek492.cpp \
        usb_492/usb492.cpp \
        usb_492/USB_Device.cpp

HEADERS  += tek492.h \
        usb_492/usb492.hpp \
        usb_492/USB_Device.hpp

FORMS    += tek492.ui

RESOURCES += tek492.qrc

unix: LIBS += -lusb

win32: LIBS += -LC:\libusb-win32-bin-1.2.6.0\libusb-win32-bin-1.2.6.0\lib\gcc -lusb
win64: LIBS += -LC:\libusb-win32-bin-1.2.6.0\libusb-win32-bin-1.2.6.0\lib\msvc_x64 -llibusb
