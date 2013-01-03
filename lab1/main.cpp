#include <QtGui/QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <iostream>

void redraw(int pos = 0, void* data = 0) {
    cv::Mat* imgIn = (cv::Mat*) data,
             imgOut;

    int cannyLow, cannyHigh;
    cannyHigh = cv::getTrackbarPos("Canny high", "Processed image");
    cannyLow = cv::getTrackbarPos("Canny low", "Processed image");

    cv::cvtColor(*(imgIn), imgOut, cv::COLOR_BGR2GRAY);
    cv::Canny(imgOut, imgOut, cannyLow, cannyHigh,3);

    cv::imshow("Processed image",imgOut);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cv::Mat imgIn;
    int cannyHigh = 200,
        cannyLow = 50;

    // load image from file
    QString dat = QFileDialog::getOpenFileName(0,"Select image");
    if (dat.isEmpty()) {
        QMessageBox::critical(0,"Error opening image", "File not selected");
        a.exit(-1);
    }
    imgIn = cv::imread(dat.toUtf8().constData());
    if (imgIn.data == NULL) {
        QMessageBox::critical(0,"Error opening image", "File not supported");
        a.exit(-1);
    }

    // prepare output windows
    cv::namedWindow("Original image", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Processed image", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Canny high","Processed image", &cannyHigh,400,redraw,(void *)&imgIn);
    cv::createTrackbar("Canny low","Processed image", &cannyLow, 400,redraw,(void *)&imgIn);
    cv::setTrackbarPos("Canny high","Processed image",cannyHigh);
    cv::setTrackbarPos("Canny low","Processed image",cannyLow);

    // display output
    cv::imshow("Original image",imgIn);
    redraw(0,&imgIn);
    
    return a.exec();
}
