#-------------------------------------------------
#
# Project created by QtCreator 2013-01-03T18:16:53
#
#-------------------------------------------------

QT       += core gui

TARGET = lab2
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

unix:LIBS = `pkg-config --libs opencv`
win32 {
    LIBS += "C:\QtSDK\mingw\lib\opencv\cv200.dll" "C:\QtSDK\mingw\lib\opencv\cvaux200.dll" "C:\QtSDK\mingw\lib\opencv\cxcore200.dll" "C:\QtSDK\mingw\lib\opencv\highgui200.dll" "C:\QtSDK\mingw\lib\opencv\ml200.dll" "C:\QtSDK\mingw\lib\opencv\opencv_ffmpeg200.dll"
    DEFINES += WIN
}

SOURCES += main.cpp
