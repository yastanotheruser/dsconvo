#ifndef DSCONVOCLIENT_H
#define DSCONVOCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include "dsconvoconnection.h"

class DSConvoClient : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Inactive,
        Error,
        Connecting,
        Connected,
        Disconnecting,
    };

    explicit DSConvoClient(QObject *parent = nullptr);
    inline const QHostAddress &address() const { return address_; }
    inline void setAddress(const QHostAddress &a) { address_ = a; }
    inline quint16 port() const { return port_; }
    inline void setPort(quint16 p) { port_ = p; }
    inline Status status() const { return status_; }
    inline const QVariant &statusData() const { return statusData_; }
    inline const QTcpSocket *socket() const { return socket_; }
    void clientConnect();
    void clientDisconnect();
    void clearError();
    QString statusString();

signals:
    void statusChanged();

private:
    void setStatus(Status status, const QVariant &data = QVariant::fromValue(nullptr));

    QTcpSocket *socket_;
    DSConvoConnection *dsconn;
    QHostAddress address_;
    quint16 port_;
    Status status_;
    QVariant statusData_;

private slots:
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);

};

#endif // DSCONVOCLIENT_H
