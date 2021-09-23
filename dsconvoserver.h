#ifndef DSCONVOSERVER_H
#define DSCONVOSERVER_H

#include <QObject>
#include <QVariant>
#include <QSet>
#include <QHash>
#include <QTcpServer>
#include <QSqlDatabase>
#include "dsconvoserverconnection.h"

class DSConvoServer : public QObject
{
    Q_OBJECT

public:
    enum State {
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

    static constexpr const char SERVER_USERNAME[] = "dsconvo";
    explicit DSConvoServer(QObject *parent = nullptr);
    ~DSConvoServer();

    inline const QHostAddress &address() const { return address_; }
    inline void setAddress(const QHostAddress &a) { address_ = a; }
    inline quint16 port() const { return port_; }
    inline void setPort(quint16 p) { port_ = p; }
    inline bool listening() const { return server->isListening(); }
    inline State state() const { return state_; }
    inline const QVariant &stateData() const { return stateData_; }

    QString stateString() const;
    bool listen();
    void close();
    void clearError();
    void broadcastMessage(const QString &message);

signals:
    void stateChanged();
    void messaged(const DSConvo::Protocol::MessageBroadcast &m);

private:
    void setState(State state, const QVariant &data = QVariant::fromValue(nullptr));
    QString makeDisplayName(const QString &username) const;
    inline void addUser(const QString &username)
    {
        users.insert(username);
        displayNames.insert(username, makeDisplayName(username));
    }

    inline bool removeUser(const QString &username)
    {
        return users.remove(username) && displayNames.remove(username);
    }

    QTcpServer *server;
    QHostAddress address_;
    quint16 port_;
    QSet<DSConvoServerConnection*> clients;
    QSet<QString> users;
    QHash<QString, QString> displayNames;
    State state_;
    QVariant stateData_;
    QSqlDatabase db;

private slots:
    void newConnection();
    void clientDataSent(const QByteArray&);
    void clientDataReceived(const QByteArray&);
    void clientDisconnected();
    void clientHelloRequested(QString &u, DSConvoServerConnection::HelloReplyError *r);
    void clientHelloAccepted();
    void clientMessageAccepted(const QString &m);
    void clientFarewellSent();

};

#endif
