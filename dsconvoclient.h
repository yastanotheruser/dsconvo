#ifndef DSCONVOCLIENT_H
#define DSCONVOCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include "dsconvoclientconnection.h"

class DSConvoClient : public QObject
{
    Q_OBJECT

public:
    enum State {
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
    inline State state() const { return state_; }
    inline const QVariant &stateData() const { return stateData_; }
    inline const QTcpSocket *socket() const { return socket_; }
    inline DSConvoClientConnection *clientConnection() const { return clientConn; }

    void clientConnect();
    void clientDisconnect();
    void clearError();
    QString stateString();

signals:
    void stateChanged();

private:
    void setState(State state, const QVariant &data = QVariant::fromValue(nullptr));

    QTcpSocket *socket_;
    DSConvoClientConnection *clientConn;
    QHostAddress address_;
    quint16 port_;
    State state_;
    QVariant stateData_;

private slots:
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);

};

#endif
