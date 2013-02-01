#include "sift.h"
#include "ui_sift.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/gpu/gpu.hpp>

sift::sift(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::sift)
{
    ui->setupUi(this);
    GPUavailable = false;
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

    *(inImage) = cv::imread(dat.toUtf8().constData(), CV_LOAD_IMAGE_GRAYSCALE);
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
    cv::VideoCapture cap(1);
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
        cv::cvtColor(frame,frame, CV_RGB2GRAY);
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

void sift::on_btnDetectObject_clicked()
{
    ui->lsStatus->clear();

    // startup info
    if (!(isLoaded(&imgObj) || isLoaded(&image1)) || !isLoaded(&image2)) {
        ui->lsStatus->addItem("Load both images before starting");
        return;
    }

    switch (ui->cbMethod->currentIndex()) {
    case 0: // SIFT
        detectOnCpu(0);
        break;
    case 1: // SURF
        detectOnCpu(1);
        break;
    case 2: // ORB
        detectOnCpu(2);
        break;
    case 3: // SURF on GPU
        if (GPUavailable)
            detectOnGpu(0);
        else
            ui->lsStatus->addItem(QString("GPU support unavailable"));
        break;
    case 4: // ORB on GPU
        if (GPUavailable)
            detectOnGpu(1);
        else
            ui->lsStatus->addItem(QString("GPU support unavailable"));
        break;
    default:
        ui->lsStatus->addItem(QString("Undefined method"));
    }
}

void sift::detectOnCpu(int method) {

    ui->lsStatus->addItem("Features extraction started");

    cv::Mat imObj, imScn;
    if (isLoaded(&imgObj)) {
        imObj = imgObj.clone();
        ui->lsStatus->addItem("Using selected object for first input");
    } else {
        imObj = image1.clone();
        ui->lsStatus->addItem("Using whole image for first input. Have you selected object?");
    }
    imScn = image2.clone();

    // pick feature detector and descriptor extractor
    cv::Feature2D *detector;
    QString detectorString("Using ");
    switch (method) {
    case 0: // SIFT
        detector = new cv::SIFT;
        detectorString.append("SIFT detector");
        break;
    case 1: // SURF
        detector = new cv::SURF(1700);
        detectorString.append("SURF detector");
        break;
    case 2: // ORB
        detector = new cv::ORB;
        detectorString.append("ORB detector");
        break;
    }
    ui->lsStatus->addItem(detectorString);

    // allocate memory
    cv::Mat descriptorObj, descriptorScn;
    QTime timer;
    int totalTime = 0;
    std::vector<cv::KeyPoint> keypointsObj, keypointsScn;

    // extract features and descriptors from object
    timer.start();
    (*detector)(imObj, cv::Mat(), keypointsObj, descriptorObj);
    totalTime += timer.elapsed();
    ui->lsStatus->addItem(QString("Object: %1 keypoints, %2 descriptors in %3ms").arg((int)keypointsObj.size()).arg(descriptorObj.rows).arg(timer.restart()));

    (*detector)(imScn, cv::Mat(), keypointsScn, descriptorScn);
    totalTime += timer.elapsed();
    ui->lsStatus->addItem(QString("Scene: %1 keypoints, %2 descriptors in %3ms").arg((int)keypointsScn.size()).arg(descriptorScn.rows).arg(timer.restart()));

    delete detector;
    // display mid-result1
    if (ui->cbFetures->isChecked()) {
        cv::Mat img_key1, img_key2;
        cv::drawKeypoints(imObj, keypointsObj, img_key1);
        cv::drawKeypoints(imScn, keypointsScn, img_key2);
        cv::namedWindow("Object keypoints");
        cv::namedWindow("Scene keypoints");
        cv::imshow("Object keypoints", img_key1);
        cv::imshow("Scene keypoints", img_key2);
    }

    // descriptor matching
    cv::Mat imgMatches;
    std::vector<cv::DMatch> matches, goodMatches, tmp;
    matches.clear();
    goodMatches.clear();

    timer.restart();
    int norm;
    if (descriptorObj.type() == CV_8U) {
        norm = cv::NORM_HAMMING; // used for ORB
    } else {
        norm = cv::NORM_L1; // used for SURF & SIFT
    }

    cv::BFMatcher matcher(norm);
    matcher.match( descriptorObj, descriptorScn, matches);

    double maxDistance = 0, minDistance = 2000;
    for (int i=0; i<(int)matches.size(); i++) {
        double dist = matches[i].distance;
        if (dist < minDistance) minDistance = dist;
        if (dist > maxDistance) maxDistance = dist;
    }
    ui->lsStatus->addItem(QString("min Distance: %1").arg(minDistance));
    ui->lsStatus->addItem(QString("max Distance: %1").arg(maxDistance));
    double matchTrh;
    if ( method == 2)
        matchTrh = minDistance * 2.3;
    else
        matchTrh = minDistance * 1.9;

    for (int i=0; i<(int)matches.size(); i++) {
        if (matches[i].distance < matchTrh) goodMatches.push_back(matches[i]);
    }

    totalTime += timer.elapsed();
    ui->lsStatus->addItem(QString("Matcher: %1 good matches out of %2 in %3ms").arg((int)goodMatches.size()).arg((int)matches.size()).arg(timer.restart()));

    // Do we have enough features to continue?
    if (goodMatches.size() < 8) {
        ui->lsStatus->addItem("Not enough features to continue");
        return;
    }

    // homography
    std::vector<cv::Point2f> obj, scn;
    obj.clear();
    scn.clear();
    for (int i=0; i<(int)goodMatches.size(); i++) {
        obj.push_back( keypointsObj[goodMatches[i].queryIdx].pt );
        scn.push_back( keypointsScn[goodMatches[i].trainIdx].pt );
    }

    cv::Mat H = cv::findHomography( obj, scn, cv::RANSAC );

    std::vector<cv::Point2f> objCorners(4), scnCorners(4);
    objCorners[0] = cvPoint(0,0);
    objCorners[1] = cvPoint(imObj.cols, 0);
    objCorners[2] = cvPoint(imObj.cols, imObj.rows);
    objCorners[3] = cvPoint(0, imObj.rows);

    cv::perspectiveTransform( objCorners, scnCorners, H);

    if (ui->cbLines->isChecked()) {
        tmp = goodMatches;
    }

    cv::drawMatches(imObj, keypointsObj, imScn, keypointsScn, tmp, imgMatches, cv::Scalar(-1), cv::Scalar(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    if (ui->cbBox->isChecked()) {
        cv::line( imgMatches, scnCorners[0] + cv::Point2f( imObj.cols, 0), scnCorners[1] + cv::Point2f( imObj.cols, 0), cv::Scalar( 250, 0, 0), 4 );
        cv::line( imgMatches, scnCorners[1] + cv::Point2f( imObj.cols, 0), scnCorners[2] + cv::Point2f( imObj.cols, 0), cv::Scalar( 250, 0, 0), 4 );
        cv::line( imgMatches, scnCorners[2] + cv::Point2f( imObj.cols, 0), scnCorners[3] + cv::Point2f( imObj.cols, 0), cv::Scalar( 250, 0, 0), 4 );
        cv::line( imgMatches, scnCorners[3] + cv::Point2f( imObj.cols, 0), scnCorners[0] + cv::Point2f( imObj.cols, 0), cv::Scalar( 250, 0, 0), 4 );
    }

    totalTime += timer.elapsed();
    ui->lsStatus->addItem(QString("Homography: %1ms").arg(timer.restart()));
    ui->lsStatus->addItem(QString("Total time: %1ms").arg(totalTime));

    cv::namedWindow("Detected object");
    cv::imshow("Detected object", imgMatches);
}

void sift::detectOnGpu(int method) {
    ui->lsStatus->addItem("Features extraction started");

    // prepare sour images and upload to GPU
    cv::Mat imObjHost;
    cv::gpu::GpuMat imObjDevice, imScnDevice;

    if (isLoaded(&imgObj)) {
        imObjDevice.upload(imgObj);
        imObjHost = imgObj.clone();
        ui->lsStatus->addItem("Using selected object for first input");
    } else {
        imObjDevice.upload(image1);
        imObjHost = image1.clone();
        ui->lsStatus->addItem("Using whole image for first input. Have you selected object?");
    }
    imScnDevice.upload(image2);

    // prepare other variabnles
    cv::gpu::GpuMat descriptorObjDevice, descriptorScnDevice,
                keypointsObjDevice, keypointsScnDevice;
    std::vector<cv::KeyPoint> keypointsObjHost, keypointsScnHost;

    QTime timer;
    int totalTime = 0;

    // perform detection and extraction
    switch (method) {
    case 0: { // SURF
        cv::gpu::SURF_GPU detector;
        detector.hessianThreshold = 1700;

        timer.start();
        detector(imObjDevice, cv::gpu::GpuMat(), keypointsObjDevice, descriptorObjDevice);
        totalTime += timer.elapsed();
        ui->lsStatus->addItem(QString("Object: %1 keypoints, %2 descriptors in %3ms").arg(keypointsObjDevice.cols).arg(descriptorObjDevice.rows).arg(timer.restart()));

        detector(imScnDevice, cv::gpu::GpuMat(), keypointsScnDevice, descriptorScnDevice);
        totalTime += timer.elapsed();
        ui->lsStatus->addItem(QString("Scene: %1 keypoints, %2 descriptors in %3ms").arg(keypointsScnDevice.cols).arg(descriptorScnDevice.rows).arg(timer.restart()));

        // download keypoints
        detector.downloadKeypoints(keypointsObjDevice, keypointsObjHost);
        detector.downloadKeypoints(keypointsScnDevice, keypointsScnHost);

        break; }
    case 1: { // ORB
        cv::gpu::ORB_GPU detector;

        timer.start();
        detector(imObjDevice, cv::gpu::GpuMat(), keypointsObjDevice, descriptorObjDevice);
        totalTime += timer.elapsed();
        ui->lsStatus->addItem(QString("Object: %1 keypoints, %2 descriptors in %3ms").arg(keypointsObjDevice.cols).arg(descriptorObjDevice.cols).arg(timer.restart()));

        detector(imScnDevice, cv::gpu::GpuMat(), keypointsScnDevice, descriptorScnDevice);
        totalTime += timer.elapsed();
        ui->lsStatus->addItem(QString("Scene: %1 keypoints, %2 descriptors in %3ms").arg(keypointsScnDevice.cols).arg(descriptorScnDevice.cols).arg(timer.restart()));

        // download keypoints
        detector.downloadKeyPoints(keypointsObjDevice, keypointsObjHost);
        detector.downloadKeyPoints(keypointsScnDevice, keypointsScnHost);

        break; }
    }

    // display mid-result1
    if (ui->cbFetures->isChecked()) {
        cv::Mat img_key1, img_key2;
        cv::drawKeypoints(imObjHost, keypointsObjHost, img_key1);
        cv::drawKeypoints(image2, keypointsScnHost, img_key2);
        cv::namedWindow("Object keypoints");
        cv::namedWindow("Scene keypoints");
        cv::imshow("Object keypoints", img_key1);
        cv::imshow("Scene keypoints", img_key2);
    }

    // descriptor matching
    std::vector<cv::DMatch> matches, goodMatches;
    matches.clear();
    goodMatches.clear();

    int norm;
    if (method == 1) {
        norm = cv::NORM_HAMMING; // used for ORB
    } else {
        norm = cv::NORM_L2; // used for SURF
    }

    cv::gpu::BFMatcher_GPU matcher(norm);

    timer.restart();
    matcher.match(descriptorObjDevice, descriptorScnDevice, matches);

    double maxDistance = 0, minDistance = 100;
    for (int i=0; i<(int)matches.size(); i++) {
        double dist = matches[i].distance;
        if (dist < minDistance) minDistance = dist;
        if (dist > maxDistance) maxDistance = dist;
    }
    ui->lsStatus->addItem(QString("min Distance: %1").arg(minDistance));
    ui->lsStatus->addItem(QString("max Distance: %1").arg(maxDistance));

    double matchTrh;
    if ( method == 0) {
        matchTrh = minDistance * 1.8;
    } else {
        matchTrh = minDistance * 1.4;
    }
    for (int i=0; i<(int)matches.size(); i++) {
        if (matches[i].distance < matchTrh) goodMatches.push_back(matches[i]);
    }

    totalTime += timer.elapsed();
    ui->lsStatus->addItem(QString("Matcher: %1 good matches out of %2 in %3ms").arg((int)goodMatches.size()).arg((int)matches.size()).arg(timer.restart()));


    // Do we have enough features to continue?
    if (goodMatches.size() < 8) {
        ui->lsStatus->addItem("Not enough features to continue");
        return;
    }

    // homography
    std::vector<cv::Point2f> obj, scn;
    obj.clear();
    scn.clear();
    for (int i=0; i<(int)goodMatches.size(); i++) {
        obj.push_back( keypointsObjHost[goodMatches[i].queryIdx].pt );
        scn.push_back( keypointsScnHost[goodMatches[i].trainIdx].pt );
    }
    cv::Mat H = cv::findHomography( obj, scn, cv::RANSAC );

    std::vector<cv::Point2f> objCorners(4), scnCorners(4);
    objCorners[0] = cvPoint(0,0);
    objCorners[1] = cvPoint(imObjDevice.cols, 0);
    objCorners[2] = cvPoint(imObjDevice.cols, imObjDevice.rows);
    objCorners[3] = cvPoint(0, imObjDevice.rows);

    cv::perspectiveTransform( objCorners, scnCorners, H);

    totalTime += timer.elapsed();
    ui->lsStatus->addItem(QString("Homography: %1ms").arg(timer.restart()));
    ui->lsStatus->addItem(QString("Total time: %1ms").arg(totalTime));

    cv::Mat imgMatches;
    cv::drawMatches(imObjHost, keypointsObjHost, image2, keypointsScnHost, goodMatches, imgMatches, cv::Scalar(-1), cv::Scalar(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    if (ui->cbBox->isChecked()) {
        cv::line( imgMatches, scnCorners[0] + cv::Point2f( imObjDevice.cols, 0), scnCorners[1] + cv::Point2f( imObjDevice.cols, 0), cv::Scalar( 250, 0, 0), 4 );
        cv::line( imgMatches, scnCorners[1] + cv::Point2f( imObjDevice.cols, 0), scnCorners[2] + cv::Point2f( imObjDevice.cols, 0), cv::Scalar( 250, 0, 0), 4 );
        cv::line( imgMatches, scnCorners[2] + cv::Point2f( imObjDevice.cols, 0), scnCorners[3] + cv::Point2f( imObjDevice.cols, 0), cv::Scalar( 250, 0, 0), 4 );
        cv::line( imgMatches, scnCorners[3] + cv::Point2f( imObjDevice.cols, 0), scnCorners[0] + cv::Point2f( imObjDevice.cols, 0), cv::Scalar( 250, 0, 0), 4 );
    }

    cv::namedWindow("Detected object");
    cv::imshow("Detected object", imgMatches);
}
