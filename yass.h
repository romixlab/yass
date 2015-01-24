#ifndef YASS_H
#define YASS_H

#include <QObject>
#include <QtRedis>

class YassPrivate;

class Yass : public QObject
{
    Q_OBJECT
public:
    explicit Yass(QObject *parent = 0);
    ~Yass();

    bool publish(QObject *object, const char *signalslot, const QString &publishPath);

signals:

private slots:
    void redisMessage(QtRedis::Reply reply);
    void catchedSignal(QString name, QVariantList args);

private:
    YassPrivate *d;

};

#endif // YASS_H
