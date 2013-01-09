#-------------------------------------------------
#
# Project created by QtCreator 2013-01-09T03:20:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lab4
TEMPLATE = app

unix:LIBS = `pkg-config --libs opencv`
win32 {
    LIBS += "C:\QtSDK\mingw\lib\opencv\cv200.dll" "C:\QtSDK\mingw\lib\opencv\cvaux200.dll" "C:\QtSDK\mingw\lib\opencv\cxcore200.dll" "C:\QtSDK\mingw\lib\opencv\highgui200.dll" "C:\QtSDK\mingw\lib\opencv\ml200.dll" "C:\QtSDK\mingw\lib\opencv\opencv_ffmpeg200.dll"
    DEFINES += WIN
}

SOURCES += main.cpp\
        sift.cpp

HEADERS  += sift.h

FORMS    += sift.ui
