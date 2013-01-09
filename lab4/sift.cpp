#include "sift.h"
#include "ui_sift.h"
#include <opencv2/opencv.hpp>
#include <QMessageBox>
#include <QFileDialog>

sift::sift(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::sift)
{
    ui->setupUi(this);
}

sift::~sift()
{
    delete ui;
}
bool sift::isLoaded(cv::Mat *inImage) {
    if (inImage->data == NULL) {
        return false;
    }
    return true;
}

void sift::loadFromFile(cv::Mat *inImage) {
    QString dat = QFileDialog::getOpenFileName(0,"Select image");
    if (dat.isEmpty()) {
        QMessageBox::critical(0,"Error opening image", "File not selected");
        return;
    }

    *(inImage) = cv::imread(dat.toUtf8().constData());
    if (!this->isLoaded(inImage)) {
        QMessageBox::critical(0,"Error opening image", "File not supported");
        return;
    }
}

void sift::showImage(cv::Mat *inImage) {
    if (!this->isLoaded(inImage)) {
        QMessageBox::information(0,"Image not loaded", "Load image before use");
        return;
    }
    cv::namedWindow("Input image");
    cv::imshow("Input image",*(inImage));
}

void sift::captureFromCamera(cv::Mat *inImage) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        QMessageBox::critical(0,"Error oppening capture source", "Unable to open default capture source");
        return;
    }

    QMessageBox::information(0,"Capture frame","Press any key to save frame");
    cv::Mat frame;
    cv::namedWindow("camera input");
    while(1) {
        cap >> frame;
        cv::imshow("camera input", frame);
        if (cv::waitKey(30) >= 0) break;
    }

    QMessageBox::StandardButton response = QMessageBox::question(0,"Use frame", "Use selected frame?",QMessageBox::Ok | QMessageBox::No | QMessageBox::Cancel , QMessageBox::Ok);
    if (response == QMessageBox::No) {
        cv::destroyWindow("camera input");
        cap.release();
        this->on_btnCamera1_clicked();
        return;
    } else if (response == QMessageBox::Ok) {
        *(inImage) = frame.clone();
    }

    cv::destroyWindow("camera input");
}

void sift::mcbSelectROI(int event, int x, int y, int, void *d) {
    cv::Mat *imgIn = (cv::Mat *)d;
    cv::Mat imgOut;
    static bool draw;
    static cv::Rect templBox;

    switch(event) {
        case cv::EVENT_LBUTTONDOWN:
            draw = true;
            templBox = cvRect(x, y, 0, 0);
            break;
        case cv::EVENT_MOUSEMOVE:
            if (draw) {
                templBox.width = x - templBox.x;
                templBox.height = y - templBox.y;
            }
            break;
        case cv::EVENT_LBUTTONUP:
            draw = false;
            if (templBox.width < 0) {
                templBox.x += templBox.width;
                templBox.width *= -1;
            }
            if (templBox.height < 0) {
                templBox.y += templBox.height;
                templBox.height *= -1;
            }

            // setROI
            *(imgIn) = cv::Mat(*(imgIn), templBox);
            cv::imshow("object",*(imgIn));

            break;
    }

    // draw selection box on image
    if (draw) {
        imgOut = imgIn->clone();
        cv::rectangle(imgOut, cv::Point(templBox.x, templBox.y), cv::Point(templBox.x + templBox.width, templBox.y + templBox.height), cv::Scalar(255,0,0));
        cv::imshow("Select object",imgOut);
    }
}

void sift::on_btnLoad1_clicked() {
    loadFromFile(&image1);
}

void sift::on_btnShow1_clicked()
{
    showImage(&image1);
}

void sift::on_btnCamera1_clicked()
{
    captureFromCamera(&image1);
}

void sift::on_btnLoad2_clicked()
{
    loadFromFile(&image2);
}

void sift::on_btnCamera2_clicked()
{
    captureFromCamera(&image2);
}

void sift::on_btnShow2_clicked()
{
    showImage(&image2);
}

void sift::on_btnROI_clicked()
{
    imgObj = this->image1.clone();
    cv::namedWindow("Select object");
    cv::setMouseCallback("Select object", mcbSelectROI, (void *)&imgObj);
    cv::imshow("Select object", imgObj);
}

void sift::on_btnShowObject_clicked()
{
    showImage(&imgObj);
}
