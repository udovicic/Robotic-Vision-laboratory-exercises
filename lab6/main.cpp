#include <QtGui/QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <stdio.h>

bool readImg(char *path, cv::Mat *depth);
bool isPlane(std::vector<float> points, std::vector<float> depth, std::vector<float> *plane);
int planePoints(cv::Mat *depth, std::vector<float> plane);

inline int rand(int min, int max) {
    return (rand() % max) + min;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // load color image
    QString dat = QFileDialog::getOpenFileName(0,"Select image");
    if (dat.isEmpty()) {
        QMessageBox::critical(0,"Error opening image", "File not selected");
        a.exit(-1);
    }

    cv::Mat img = cv::imread(dat.toUtf8().constData());
    if (img.data == NULL) {
        QMessageBox::critical(0,"Error opening image", "File not supported");
        a.exit(-1);
    }

    // load Kinect depth map
    cv::Mat img_depth(img.rows, img.cols, CV_8UC1);
    dat.chop(4); dat.append("-D.txt");
    if (readImg(dat.toUtf8().data(), &img_depth) == 0) {
        QMessageBox::critical(0, "Error opening depth map", "File incorret");
        a.exit(-1);
    }

    // use RANSAC to determine dominant plane
    srand(time(NULL));
    int ransac_runds = 1000,
            bestPlane = 0;
    std::vector<float> bPlane;

    for (int i=0; i<ransac_runds; i++) {
        std::vector<float> points, depth, plane;
        points.clear(); depth.clear(); plane.clear();

        for (int j=0; j<3; j++) {
            int u = rand(0, img.cols),
                    v = rand(0, img.cols),
                    d = img_depth.at<uchar>(v,u);

            points.push_back((float)u);
            points.push_back((float)v);
            points.push_back(1.0f);

            depth.push_back((float)d);
        }

        if (!isPlane(points, depth, &plane))
            continue; // invalid point, skip iteration

        int num = planePoints(&img_depth, plane);
        if (num > bestPlane) {
            bestPlane = num;
            bPlane.swap(plane);
        }

    }

    // mark result
    int width = img_depth.cols,
            height = img_depth.rows;
    cv::cvtColor(img_depth, img_depth, cv::COLOR_GRAY2RGB);

    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            uchar d = img_depth.at<cv::Vec3b>(j,i)[0];
            float err = (float)d - (bPlane[0]*i + bPlane[1]*j + bPlane[2]);
            if (err < 0) err *= -1;
            if (err < 4.0f) {
                img_depth.at<cv::Vec3b>(j,i)[2] = 0; // remove red
                img_depth.at<cv::Vec3b>(j,i)[1] = 0; // remove green
            }
        }
    }



    // output result
    cv::imshow("Original", img);
    cv::imshow("Depth", img_depth);

    return a.exec();
}

bool readImg(char *path, cv::Mat *depth) { // Read Kinect image
    FILE *file = fopen(path, "r");
    if (file == NULL) return 0;

    int width = depth->cols, height = depth->rows,
        map[width * height], // temp field for reading
        min = 2047, max = 0; // (min,max) -> (0,255)

    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            int d;
            map[j*width + i] = 0; // clear memory

            while (fscanf(file, "%d ", &d) != 1);
//            if (fscanf(file, "%d ", &d) != 1) break;

            if (d==2047)
                d=-1; // Invalid depth (unable to measure)
            else {
                if (d>max) max=d;
                if (d<min) min=d;
            }

            map[j*width + i] = d; // temp storage
        }
    }

    fclose(file);

    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

            int d = map[j*width + i]; // read from temp storage
            if (d >= min)
                d = ((d-min) * 254 / (max-min)) + 1; // scale to fit
            else
                d = 0;
            depth->at<uchar>(j,i) = d; // write to output matrix
        }
    }

    return 1;
}

bool isPlane(std::vector<float> points, std::vector<float> depth, std::vector<float> *plane) {

    cv::Mat A_(3, 3, CV_32FC1, &points[0]);
    cv::Mat Z_(3, 1, CV_32FC1, &depth[0]);
    cv::Mat p_(3, 1, CV_32FC1);

    if (cv::solve(A_, Z_, p_)) {
        for (int i=0; i<3; i++) plane->push_back(p_.at<float>(i));
        return 1; // valid points
    } else
        return 0; // selected points are on the same line
}

int planePoints(cv::Mat *depth, std::vector<float> plane) {
    float thr = 4.0f; // threshold
    int nMatch = 0; // number of matches


    for (int j=0; j<depth->rows; j++) {
        for (int i=0; i<depth->cols; i++) {
            uchar d = depth->at<uchar>(j,i);
            float err = (float)d - (plane[0]*i + plane[1]*j + plane[2]);
            if (err < 0) err *= -1;
            if (err > thr)
                continue;
            else
                nMatch++;
        }
    }

    return nMatch;
}
