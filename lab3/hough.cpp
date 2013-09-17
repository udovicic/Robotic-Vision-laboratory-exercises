#include "hough.h"
#include "ui_hough.h"
#include <fstream>

hough::hough(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::hough)
{
    ui->setupUi(this);
    ready = false;
}

hough::~hough()
{
    delete ui;
}

void hough::on_btnLoadImage_clicked()
{
    // load image from file
    QString dat = QFileDialog::getOpenFileName(0,"Select image");
    if (dat.isEmpty()) {
        QMessageBox::critical(0,"Error opening image", "File not selected");
        return;
    }
    img = cv::imread(dat.toUtf8().constData());
    if (img.data == NULL) {
        QMessageBox::critical(0,"Error opening image", "File not supported");
        ready = false;
        return;
    }
    // set ready flag
    ready = true;
    original = img.clone();
}

void hough::on_btnShow_clicked()
{
    if (ready == true) {
        cv::namedWindow("Original image", cv::WINDOW_AUTOSIZE);
        cv::imshow("Original image", img);
    } else {
        QMessageBox::critical(0,"Image not loaded", "Image not loaded");
    }
}

void hough::on_btnLoadParam_clicked()
{
    QString dat;
    cv::FileStorage fs;

    if (ready != true) {
        QMessageBox::critical(0,"Image not loaded", "Image not loaded");
        return;
    }

    // Open camera parameters
    std::ifstream intr("camera.yml", std::ios::in);
    if (intr.is_open()) {   // Check in running dir first
         fs.open("camera.yml", cv::FileStorage::READ);
    } else {    // No?? Browse for it....
        dat = QFileDialog::getOpenFileName(this,"Locate camera.yml");
        if (dat.isEmpty()) {
            QMessageBox::warning(this,"File not found","File camera.yml not loaded!");
            return;
        }
        fs.open(dat.toUtf8().constData(), cv::FileStorage::READ);
    }
    fs["intrinsics"] >> intrinsic;
    fs["distortion"] >> distortion;
    fs.release();

    // make clone of original image and prepare it
    cv::Mat mapx, mapy;
    mapx = img.clone();
    mapy = img.clone();

    cv::initUndistortRectifyMap(intrinsic, distortion, cv::Mat(), intrinsic, img.size(), CV_32FC1, mapx, mapy);
    cv::remap(img, img, mapx, mapy,cv::INTER_NEAREST,cv::BORDER_CONSTANT,cv::Scalar());
    original = img.clone();
    QMessageBox::information(0,"Undistorted", "image has been undistorted");
}

void hough::mcbSelectROI(int event, int x, int y, int, void *d) {
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
            cv::destroyAllWindows();
            return;

            break;
    }

    // draw selection box on image
    if (draw) {
        imgOut = imgIn->clone();
        cv::rectangle(imgOut, cv::Point(templBox.x, templBox.y), cv::Point(templBox.x + templBox.width, templBox.y + templBox.height), cv::Scalar(255,0,0));
        cv::imshow("Select area",imgOut);
    }
}

void hough::on_btnSelectROI_clicked()
{
    if (ready == true) {
        img = original.clone();
        cv::namedWindow("Select area");
        cv::setMouseCallback("Select area", mcbSelectROI, (void *)&img);
        cv::imshow("Select area", img);
    } else {
        QMessageBox::critical(0,"Image not loaded", "Image not loaded");
    }
}

void hough::on_btnDetect_clicked()
{
    //temp image
    cv::Mat tempImg,
            imgOut = img.clone(),
            // image parameters
            rvec = cv::Mat(1,3,CV_32FC1),
            tvec = cv::Mat(1,3,CV_32FC1);

    float m[4][3] = {{0.0f, 0.0f, 0.0f}, {490.0f, 0.0f, 0.0f}, {490.0f, 310.0f, 0.0f}, {0.0f, 310.0f, 0.0f}};
    cv::Mat objectPoints = cv::Mat(4, 3, CV_32FC1, m).clone();
    cv::Size size = img.size();
    float s[4][2] = {{0.0f, 0.0f},{(float)size.width, 0.0f}, {(float)size.width, (float)size.height}, {0.0f, (float)size.height}};
    cv::Mat imagePoints = cv::Mat(4, 2, CV_32FC1, s).clone();

    // prepare image
    cv::cvtColor(img, tempImg, cv::COLOR_BGR2GRAY);
    cv::Canny(tempImg, tempImg, 100, 300, 3);

    std::vector<cv::Vec2f> lines;
    cv::HoughLines( tempImg, lines, 1, CV_PI/180, 100 );

    if (lines.size() < 1) {
        QMessageBox::critical(0,"No lines", "No lines detected");
        return;
    }

    double lambX, lambY, lambRho;
    cv::Mat A = cv::Mat(3, 3, CV_32FC1),
            B = cv::Mat(3, 1, CV_32FC1),
            R = cv::Mat(3, 3, CV_32FC1);
    float rho = lines[0][0];
    float theta = lines[0][1];
    double a = cos(theta), b = sin(theta);
    double x0 = a*rho, y0 = b*rho;
    cv::Point pt1(cvRound(x0 + 1000*(-b)),
              cvRound(y0 + 1000*(a)));
    cv::Point pt2(cvRound(x0 - 1000*(-b)),
              cvRound(y0 - 1000*(a)));
    cv::line( imgOut, pt1, pt2, cv::Scalar(0,0,255), 3, 8 );

    cv::solvePnP(objectPoints, imagePoints, intrinsic, distortion, rvec, tvec);
    cv::Rodrigues(rvec, R);

    R.convertTo(R, CV_32FC1);
    tvec.convertTo(tvec, CV_32FC1);
    A = intrinsic * R;
    B = intrinsic * tvec;
    lambX = A.at<float>(0,0) * cos(theta) + A.at<float>(1,0) * sin(theta) - rho * A.at<float>(2,0);
    lambY = A.at<float>(0,1) * cos(theta) + A.at<float>(1,1) * sin(theta) - rho * A.at<float>(2,1);
    lambRho = B.at<float>(2) * rho - B.at<float>(0) * cos(theta) - B.at<float>(1) * sin(theta);
    theta = atan2(lambY, lambX);
    rho = lambRho / sqrt(lambX*lambX + lambY*lambY);

    // Prepare and print text
    char text[65];
    cv::Point location;
    location.x = size.width/2;
    location.y = size.height/2;
    sprintf(text,"Theta: %6.2f [deg]", theta*180/3.141592);
    cv::putText(imgOut, text, location, CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar::all(255), 1);
    location.y += 20;
    sprintf(text,"Rho: %6.2f [mm]", rho);
    cv::putText(imgOut, text, location, CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar::all(255), 1);

    cv::imshow("lines", imgOut);
    cv::imwrite("input.png", img);
    cv::imwrite("output.png", imgOut);
}
