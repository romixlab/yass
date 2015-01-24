#include "yass.h"
#include <qmsgpack/msgpack.h>
#include "dqobject.h"
#include <QDebug>

class YassPrivate {
    QtRedis *redis;
    DQobject *dq;
    friend class Yass;
};

Yass::Yass(QObject *parent) :
    QObject(parent), d(new YassPrivate)
{
    d->dq = new DQobject(this);
    connect(d->dq, SIGNAL(catched(QString,QVariantList)),
            this, SLOT(catchedSignal(QString,QVariantList)));

    d->redis = new QtRedis("10.0.1.1", 6379, this);
    qDebug() << d->redis->openConnection();
}

Yass::~Yass()
{
    delete d;
}

bool Yass::publish(QObject *object, const char *signalslot, const QString &publishPath)
{
    qDebug() << "publishing" << publishPath;
    if (signalslot[0] == '2') { // publishing signal
        // @todo need unique names based on arg types
        QString dsignalName = QString("%1%2").arg(publishPath).arg(signalslot);
        qDebug() << "dsignal" << dsignalName;
        qDebug() << d->dq->addSlot(dsignalName);
        qDebug() << connect(object, signalslot,
                d->dq, qPrintable(QString("1%1").arg(dsignalName)));
    }
}

void Yass::redisMessage(QtRedis::Reply reply)
{
    qDebug() << "Channel:" << reply.channel << "Pattern:" << reply.pattern;
    qDebug() << reply.value.toString();
}

void Yass::catchedSignal(QString name, QVariantList args)
{
    qDebug() << "catched" << name << args;
    QString data(MsgPack::pack(args).toBase64());
    d->redis->publish("foo", data);
}
