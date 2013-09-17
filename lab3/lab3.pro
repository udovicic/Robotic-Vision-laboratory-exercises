#-------------------------------------------------
#
# Project created by QtCreator 2013-09-17T17:22:31
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lab3
TEMPLATE = app

unix:LIBS = `pkg-config --libs opencv`
win32 {
    LIBS += "C:\QtSDK\mingw\lib\opencv\cv200.dll" "C:\QtSDK\mingw\lib\opencv\cvaux200.dll" "C:\QtSDK\mingw\lib\opencv\cxcore200.dll" "C:\QtSDK\mingw\lib\opencv\highgui200.dll" "C:\QtSDK\mingw\lib\opencv\ml200.dll" "C:\QtSDK\mingw\lib\opencv\opencv_ffmpeg200.dll"
    DEFINES += WIN
}

SOURCES += main.cpp\
        hough.cpp

HEADERS  += hough.h

FORMS    += hough.ui
