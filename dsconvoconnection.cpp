#include "dsconvoconnection.h"
#include <QtDebug>
#include <limits>

DSConvoConnection::DSConvoConnection(QTcpSocket *socket, QObject *parent)
    : QObject(parent)
    , socket_(socket)
    , stream(new DSConvoStream(this))
{
    qDebug("[DEBUG] DSConvoConnection::DSConvoConnection(%p)", socket);
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(stream, SIGNAL(messageParsed(DSConvoStream::ParsedMessage&)), this,
            SLOT(streamMessageParsed(DSConvoStream::ParsedMessage&)));
}

DSConvoConnection::~DSConvoConnection()
{
    delete socket_;
}

quint64 DSConvoConnection::send(const QByteArray &data)
{
    quint32 dataSize = data.size();
    Q_ASSERT(dataSize <= std::numeric_limits<unsigned short>::max());
    auto sent = socket_->write(reinterpret_cast<const char*>(&dataSize), 2);
    sent += socket_->write(data);
    emit dataSent(data);
    return sent;
}

void DSConvoConnection::socketReadyRead()
{
    QByteArray data = socket_->readAll();
    qDebug("[DEBUS] DSConvoConnection::socketReadyRead() (%d bytes)", data.size());
    emit dataReceived(data);
    stream->handleRecvData(data);
}
