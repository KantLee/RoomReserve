#include <QtWidgets/QApplication>

#include "kroomreserve.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KRoomReserve w;
    w.show();
    return a.exec();
}
