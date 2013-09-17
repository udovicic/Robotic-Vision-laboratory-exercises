#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <opencv2/opencv.hpp>

int templMethod;
float templThresh;
cv::Rect templBox;
cv::Mat imgIn;

void templateMatch() {
    // extract object from image
    cv::Mat object = cv::Mat(imgIn, templBox);
    cv::imshow("Object", object);

    // create raw template match
    cv::Size inSize = imgIn.size(),
            objSize = object.size();
    inSize.width = inSize.width - objSize.width + 1;
    inSize.height = inSize.height - objSize.height + 1;

    cv::Mat raw(inSize, CV_32FC1, NULL);
    cv::matchTemplate(imgIn, object, raw, templMethod);
    // invert raw output if needed
    if (templMethod == cv::TM_SQDIFF_NORMED) cv::subtract(cv::Mat::ones(raw.size(),CV_32FC1),raw,raw);

    cv::imshow("RAW output", raw);

    // indicate template matches
    cv::Mat imgOut = imgIn.clone();
    float thresh = templThresh;
    int windowSize = 11;
    int windowRad = (int)(windowSize/2);
    float centerPixelValue, currentPixelValue;
    bool bcenterPixelMax = false;
    uchar *pImageData = raw.data;

    //center pixel must be greater than threshold and all pixels in subwindow less than center pixel.
    //if only one pixel is greater than center pixel discard that window
    for (int y=0; y<inSize.height; y++) {
        for (int x=0; x<inSize.width; x++) {
            //subwindow
            if((x - windowRad >= 0) &&
                    (x + windowRad <= inSize.width-1) &&
                    (y - windowRad >= 0) &&
                    (y + windowRad <= inSize.height-1)
                    ) {
                centerPixelValue = ((float *)(pImageData + raw.step * y))[x];

                if(centerPixelValue > thresh) {
                    bcenterPixelMax = true;

                    for (int j = y-windowRad; j <= y+windowRad; j++) {
                        for (int i = x - windowRad; i <= x+windowRad; i++) {
                            currentPixelValue = ((float *)(pImageData + raw.step * j))[i];

                            if(currentPixelValue > centerPixelValue) {
                                bcenterPixelMax = false;
                                break; // exit i
                            }
                        }
                        if(!bcenterPixelMax) break;  //exit j
                    }
                    // draw box arround pixel
                    if(bcenterPixelMax) cv::rectangle(imgOut, cv::Point(x,y), cv::Point(x + objSize.width, y + objSize.height), cv::Scalar(255,0,255));
                }
            }
        }
    }

    cv::imshow("Result", imgOut);
}

void mcbSelectROI(int event, int x, int y, int, void *) {
    cv::Mat imgOut;
    static bool draw;

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

            // call template matching function
            templateMatch();
            break;
    }

    // draw selection box on image
    if (draw) {
        imgOut = imgIn.clone();
        cv::rectangle(imgOut, cv::Point(templBox.x, templBox.y), cv::Point(templBox.x + templBox.width, templBox.y + templBox.height), cv::Scalar(255,0,0));
        cv::imshow("Original image",imgOut);
    }
}

void btnClick(int status, void *data) {
    char* dat = (char *)data;

    if (status) {
        if (dat[7] == 'Q') templMethod = cv::TM_SQDIFF_NORMED;
        if (dat[7] == 'O') templMethod = cv::TM_CCORR_NORMED;
        if (dat[7] == 'C') templMethod = cv::TM_CCOEFF_NORMED;
    }
    if (dat[7] == 'l') templateMatch();
}

void tbClick(int value, void *) {
    templThresh = (float)value / 100;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
    templMethod = cv::TM_CCOEFF_NORMED;
    templThresh = 0.7f;
    int val = 80;

    cv::namedWindow("Original image", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Original image", mcbSelectROI, NULL);
    char    *btn1 = "CV_TM_SQDIFF",
            *btn2 = "CV_TM_CORR",
            *btn3 = "CV_TM_CCOEFF",
            *btn4 = "Recalculate";
    cv::createButton(btn1, btnClick, btn1, cv::QT_RADIOBOX, 1);
    cv::createButton(btn2, btnClick, btn2, cv::QT_RADIOBOX, 0);
    cv::createButton(btn3, btnClick, btn3, cv::QT_RADIOBOX, 0);
    cv::createTrackbar("Threshold", "", &val, 100, tbClick, 0);
    cv::createButton(btn4, btnClick, btn4);

    // show images
    cv::imshow("Original image", imgIn);

    return a.exec();
}
