#include "dsconvoconnection.h"
#include <QtDebug>

DSConvoConnection::DSConvoConnection(QTcpSocket *socket, QObject *parent)
    : QObject(parent)
    , socket_(socket)
    , stream(new DSConvoStream(this))
{
    qDebug("[DEBUG] DSConvoConnection::DSConvoConnection(%p)", socket);
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
}

DSConvoConnection::~DSConvoConnection()
{
    delete socket_;
}

quint64 DSConvoConnection::send(const QByteArray &data)
{
    quint64 written = socket_->write(data);
    emit dataSent(data);
    return written;
}

void DSConvoConnection::socketReadyRead()
{
    QByteArray data = socket_->readAll();
    qDebug("[DEBUS] DSConvoConnection::socketReadyRead() (data : %s)",
           data.toHex().constData());
    emit dataReceived(data);
    stream->handleRecvData(data);
}
