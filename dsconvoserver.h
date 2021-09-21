#ifndef DSCONVOSERVER_H
#define DSCONVOSERVER_H

#include <QObject>
#include <QVariant>
#include <QSet>
#include <QTcpServer>
#include "dsconvoconnection.h"

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

    explicit DSConvoServer(const QHostAddress &address = QHostAddress::AnyIPv4,
                           quint16 port = DSConvo::DEFAULT_PORT,
                           QObject *parent = nullptr);
    ~DSConvoServer();
    inline const QHostAddress &address() const { return address_; }
    inline quint16 port() const { return port_; }
    inline bool listening() const { return server->isListening(); }
    inline Status status() const { return status_; }
    inline const QVariant &statusData() const { return statusData_; }
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
    void clientDataSent(const QByteArray&);
    void clientDataReceived(const QByteArray&);
    void clientDisconnected();

};

#endif // DSCONVOSERVER_H
