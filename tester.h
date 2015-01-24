#ifndef TESTER_H
#define TESTER_H

#include <QObject>

class Tester : public QObject
{
    Q_OBJECT
public:
    explicit Tester(QObject *parent = 0);

    void emitSomeSignal() {
        emit someSignal(123, "some string");
    }

signals:
    void someSignal(int val, const QString &str);

public slots:

};

#endif // TESTER_H
