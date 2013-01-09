#include <QApplication>
#include "sift.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    sift w;
    w.show();
    
    return a.exec();
}
