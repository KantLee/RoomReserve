#include <QtWidgets/QApplication>

#include "kroomreserveunittest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KRoomReserveUnitTest roomReserveUnitTest;
    QTest::qExec(&roomReserveUnitTest, argc, argv);
    return 0;
}
