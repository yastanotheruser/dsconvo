#ifndef DSCONVOCONNECTION_H
#define DSCONVOCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include "dsconvocommon.h"
#include "dsconvostream.h"

class DSConvoConnection : public QObject
{
    Q_OBJECT

public:
    explicit DSConvoConnection(QTcpSocket *socket, QObject *parent = nullptr);
    virtual ~DSConvoConnection();
    inline const QTcpSocket *socket() const { return socket_; }

signals:
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void disconnected();

protected:
    quint64 send(const QByteArray &data);
    QTcpSocket *socket_;
    DSConvoStream *const stream;

protected slots:
    virtual void streamMessageParsed(DSConvoStream::ParsedMessage&)
    {
    }

private slots:
    void socketReadyRead();

};

#endif
