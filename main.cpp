#include <QCoreApplication>
#include <QtRedis>
#include <yass.h>

#include "tester.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Yass yass;

    Tester tester;
    yass.publish(&tester, SIGNAL(someSignal(int,QString)), "somesig");
    tester.emitSomeSignal();



    return a.exec();
}
