#ifndef SIFT_H
#define SIFT_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>

namespace Ui {
class sift;
}

class sift : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit sift(QWidget *parent = 0);
    ~sift();

    // static mouse callback
    static void mcbSelectROI(int event, int x, int y, int, void *d);
    
private slots:
    void on_btnLoad1_clicked();

    void on_btnShow1_clicked();

    void on_btnCamera1_clicked();

    void on_btnLoad2_clicked();

    void on_btnCamera2_clicked();

    void on_btnShow2_clicked();

    void on_btnROI_clicked();

    void on_btnShowObject_clicked();

private:
    Ui::sift *ui;

    // shared OpenCV variables
    cv::Mat image1, image2, imgObj;


    // shared functions
    bool isLoaded(cv::Mat *inImage);
    void loadFromFile(cv::Mat *inImage);
    void captureFromCamera(cv::Mat *inImage);
    void showImage(cv::Mat *inImage);
};

#endif // SIFT_H
