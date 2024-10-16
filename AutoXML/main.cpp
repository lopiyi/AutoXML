#include "autoxml.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AutoXML w;
    w.show();
    return a.exec();
}
