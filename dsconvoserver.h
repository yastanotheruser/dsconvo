#ifndef DSCONVOSERVER_H
#define DSCONVOSERVER_H

#include <QObject>
#include <QVariant>
#include <QSet>
#include <QTcpServer>
#include "dsconvoconnection.h"

typedef QPair<QAbstractSocket::SocketError, QString> SocketErrorInfo;
Q_DECLARE_METATYPE(SocketErrorInfo);

class DSConvoServer : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Inactive,
        Waiting,
        Error,
        Listening,
        Connection,
        Disconnection,
        Send,
        Recv,
        Closing,
    };

    static constexpr quint16 DEFAULT_PORT = 5500;
    DSConvoServer(const QHostAddress &address = QHostAddress::AnyIPv4,
                  quint16 port = DEFAULT_PORT, QObject *parent = nullptr);
    ~DSConvoServer();
    const QHostAddress &address() const;
    quint16 port() const;
    bool listening() const;
    Status status() const;
    const QVariant &statusData() const;
    bool listen();
    void close();
    void clearError();
    QString statusString();

signals:
    void statusChanged();

private:
    void setStatus(Status status, const QVariant &data = QVariant::fromValue(nullptr));

    QTcpServer *server;
    QHostAddress const address_;
    quint16 const port_;
    QSet<DSConvoConnection*> clients;
    Status status_;
    QVariant statusData_;

private slots:
    void newConnection();
    void clientDataSent(const QByteArray &data);
    void clientDataReceived(const QByteArray &data);
    void clientDisconnected();
};

#endif // DSCONVOSERVER_H
