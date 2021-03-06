#include "dqobject.h"
#include "qmetaobjectbuilder_p.h"
#include <QDebug>

class DQobjectPrivate {
    QMetaObject *meta;
    QByteArray metadata;
    int catchedSignalId;
    QHash<int, QString> slotId2Name;
    QHash<QString, int> signalName2Id;
//    QHash<QString, QVector<QMetaObject::Connection> > signalConnections;
    friend class DQobject;
};

DQobject::DQobject(QObject *parent) :
    QObject(parent), d(new DQobjectPrivate)
{
    QMetaObjectBuilder builder;
    builder.setClassName("DQobject");
    d->catchedSignalId = builder.addSignal("catched(QString,QVariantList)").index();
    d->meta = builder.toMetaObject();

    QDataStream ostream(&d->metadata, QIODevice::WriteOnly);
    builder.serialize(ostream);
}

DQobject::~DQobject()
{
    if (d->meta)
        free(d->meta);
    delete d;
}

bool DQobject::addSlot(const QString &slot)
{
    if (d->slotId2Name.key(slot, -1) != -1)
        return false;

    QDataStream istream(d->metadata);
    QMetaObjectBuilder builder;
    QMap<QByteArray, const QMetaObject*> refs;
    builder.deserialize(istream, refs);

    int metaIndex = builder.addSlot(slot.toLocal8Bit()).index();
    d->slotId2Name.insert(metaIndex + d->meta->methodOffset(), slot);

    if (d->meta)
        free(d->meta);
    d->meta = builder.toMetaObject();
    QDataStream ostream(&d->metadata, QIODevice::WriteOnly);
    builder.serialize(ostream);

    return true;
}

bool DQobject::removeSlot(const QString &slot)
{
    if (d->slotId2Name.key(slot, -1) == -1)
        return false;

    QDataStream istream(d->metadata);
    QMetaObjectBuilder builder;
    QMap<QByteArray, const QMetaObject*> refs;
    builder.deserialize(istream, refs);

    int metaIndex = d->slotId2Name.key(slot);
    builder.removeMethod(metaIndex - d->meta->methodOffset());
    d->slotId2Name.remove(metaIndex);

    if (d->meta)
        free(d->meta);
    d->meta = builder.toMetaObject();
    QDataStream ostream(&d->metadata, QIODevice::WriteOnly);
    builder.serialize(ostream);

    return true;
}

bool DQobject::addSignal(const QString &signal)
{
    if (d->signalName2Id.value(signal, -1) != -1)
        return false;

    QDataStream istream(d->metadata);
    QMetaObjectBuilder builder;
    QMap<QByteArray, const QMetaObject*> refs;
    builder.deserialize(istream, refs);

    int metaIndex = builder.addSignal(signal.toLocal8Bit()).index()
            + d->meta->methodOffset();
    d->signalName2Id.insert(signal, metaIndex);

    if (d->meta)
        free(d->meta);
    d->meta = builder.toMetaObject();
    QDataStream ostream(&d->metadata, QIODevice::WriteOnly);
    builder.serialize(ostream);

    return true;
}

void DQobject::activateSignal(const QString &signal, const QVariantList &args)
{
    int metaIndex = d->signalName2Id.value(signal, -1);
    if (metaIndex == -1) {
        qWarning() << "dynamic signal" << signal << "doesn't exist";
        return;
    }

    QMetaMethod sig = d->meta->method(metaIndex);
    if (sig.parameterCount() != args.count()) {
        qWarning() << "dynamic signal has" << sig.parameterCount() << "args"
                   << args.count() << "provided";
        return;
    }

    QVariantList argsCopy = args;
    void * _a[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < sig.parameterCount(); ++i) {
        if (!argsCopy[i].convert(sig.parameterType(i))) {
            qWarning() << "dynamic signal" << signal << "wrong parameter" << i;
            return;
        }
        _a[i + 1] = argsCopy[i].data();
    }

    QMetaObject::activate(this, d->meta, metaIndex - d->meta->methodOffset(), _a);
}

bool DQobject::removeSignal(const QString &signal)
{
    if (d->signalName2Id.value(signal, -1) == -1)
        return false;

    QDataStream istream(d->metadata);
    QMetaObjectBuilder builder;
    QMap<QByteArray, const QMetaObject*> refs;
    builder.deserialize(istream, refs);

    int metaIndex = d->signalName2Id.value(signal) - d->meta->methodOffset();
    d->signalName2Id.remove(signal);
    builder.removeMethod(metaIndex);

    if (d->meta)
        free(d->meta);
    d->meta = builder.toMetaObject();
    QDataStream ostream(&d->metadata, QIODevice::WriteOnly);
    builder.serialize(ostream);

    return true;
}

const QMetaObject *DQobject::metaObject() const
{
    return d->meta;
}

int DQobject::qt_metacall(QMetaObject::Call call, int id, void **args)
{
    id = QObject::qt_metacall(call, id, args);
    if (id < 0 || !d->meta)
        return id;

    if (call == QMetaObject::InvokeMetaMethod) {
        int metaIndex = id + d->meta->methodOffset();
        QMetaMethod method = d->meta->method(metaIndex);
        if (!method.isValid()) {
            qWarning() << "invoked invalid method with id" << id;
            return -1;
        }
        QVariantList vargs;
        for (int i = 0; i < method.parameterCount(); ++i)
            vargs.append(QVariant(method.parameterType(i), args[1 + i]));
        QString catchedName = d->slotId2Name.value(metaIndex);
        void *_a[] = { 0,
                       const_cast<void*>(reinterpret_cast<const void*>(&catchedName)),
                       const_cast<void*>(reinterpret_cast<const void*>(&vargs)) };
        QMetaObject::activate(this, metaObject(), d->catchedSignalId, _a);
    }

    return -1;
}

void *DQobject::qt_metacast(const char *className)
{
    if (!className) return 0;
    //this object should not be castable to anything but QObject
    return QObject::qt_metacast(className);
}

//bool DQobject::connect(const QObject *sender, const char *signal,
//                   const QObject *receiver, const char *member, Qt::ConnectionType type)
//{
//    if ((sender != this) && (receiver != this)) {
//        qWarning() << "don't use DQObject::connect for normal connections!";
//        return false;
//    }

//    if (sender == this) { // connection to our signal
//        QMetaObject::Connection connection =
//                QObject::connect(sender, signal, receiver, member, type);
//        if (!connection)
//            return false;
//        d->signalConnections.insert(&signal[1], connection);
//    }
//}


//void DQobject::disconnect_signal(const QString &signal)
//{
//    qDebug() << "signal" << signal << "removed, disconnecting";


//}
