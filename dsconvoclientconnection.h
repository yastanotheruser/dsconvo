#ifndef DSCONVOCLIENTCONNECTION_H
#define DSCONVOCLIENTCONNECTION_H

#include "dsconvoconnection.h"
#include <QObject>

class DSConvoClientConnection : public DSConvoConnection
{
    Q_OBJECT

public:
    enum State {
        Ungreeted,
        Greeting,
        Greeted,
    };

    enum Error {
        BadUsername,
        RepeatedUsername,
        ServerLoad,
    };

    explicit DSConvoClientConnection(QTcpSocket *socket, QObject *parent = nullptr);
    inline State state() const { return state_; }
    inline const QString &username() const { return username_; }
    void sendHello(const QString &username);
    void sendMessage(const QString &message);
    void sendGoodbye();

signals:
    void greeted();
    void messaged(const DSConvo::Protocol::MessageBroadcast &m);
    void errorOccurred(DSConvoClientConnection::Error);

private:
    void handleHelloReply(const google::protobuf::Message*);
    void handleMessageBroadcast(const google::protobuf::Message*);
    State state_;
    QString username_;

protected slots:
    void streamMessageParsed(DSConvoStream::ParsedMessage&) override;

};

#endif
