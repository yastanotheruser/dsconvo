#ifndef DSCONVOCONNECTION_H
#define DSCONVOCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include "dsconvostream.h"

class DSConvoConnection : public QObject
{
    Q_OBJECT

public:
    explicit DSConvoConnection(QTcpSocket *socket, QObject *parent = nullptr);
    ~DSConvoConnection();
    inline const QTcpSocket *socket() const { return socket_; }

signals:
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void disconnected();

private:
    quint64 send(const QByteArray &data);
    QTcpSocket *socket_;
    DSConvoStream *const stream;

private slots:
    void socketReadyRead();

};

#endif // DSCONVOCONNECTION_H
