#include "hough.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    hough w;
    w.show();
    
    return a.exec();
}
