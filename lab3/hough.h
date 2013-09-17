#ifndef HOUGH_H
#define HOUGH_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <opencv2/opencv.hpp>

namespace Ui {
class hough;
}

class hough : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit hough(QWidget *parent = 0);
    ~hough();

    // static mouse callback
    static void mcbSelectROI(int event, int x, int y, int, void *d);
    
private slots:
    void on_btnLoadImage_clicked();

    void on_btnShow_clicked();

    void on_btnLoadParam_clicked();

    void on_btnSelectROI_clicked();

    void on_btnDetect_clicked();

private:
    Ui::hough *ui;
    cv::Mat img, original;
    cv::Mat intrinsic, distortion;
    bool ready;
};

#endif // HOUGH_H
