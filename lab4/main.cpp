#include <QApplication>
#include <opencv2/gpu/gpu.hpp>
#include "sift.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    sift w;

    int ndev = 0;
    try {
        ndev = cv::gpu::getCudaEnabledDeviceCount();
        if (ndev > 0) {
            w.GPUavailable = true;
        }
    } catch (cv::Exception& e) {
        w.GPUavailable = false;
        std::cerr << e.what() << std::endl;
    }

    w.show();
    
    return a.exec();
}
