#include <QtGui/QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <opencv2/opencv.hpp>

void btn1click(int state, void *data) {
    if (state) {
        char *btn = (char*)data;
        QMessageBox::information(0,"info",QString(btn));
    }
}

void mcbSelectROI(int event, int x, int y, int flags, void *data) {

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    cv::Mat imgIn;

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

    // prepare GUI
    cv::namedWindow("Original image",cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Original image",mcbSelectROI,NULL);
    char    *btn1 = "CV_TM_SQDIFF",
            *btn2 = "CV_TM_CORR",
            *btn3 = "CV_TM_CCOEFF";
    cv::createButton(btn1,btn1click,btn1,CV_RADIOBOX,1);
    cv::createButton(btn2,btn1click,btn2,CV_RADIOBOX,0);
    cv::createButton(btn3,btn1click,btn3,CV_RADIOBOX,0);

    // show images
    cv::imshow("Original image",imgIn);

    //while(1) if(cv::waitKey(30) >= 0) break;

    return a.exec();
}
