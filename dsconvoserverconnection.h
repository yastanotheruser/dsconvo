#ifndef DSCONVOSERVERCONNECTION_H
#define DSCONVOSERVERCONNECTION_H

#include "dsconvoconnection.h"
#include <QObject>
#include <functional>

class DSConvoServerConnection : public DSConvoConnection
{
    Q_OBJECT

public:
    using HelloReplyError = DSConvoProtocol::HelloReplyPayload::HelloReplyError;

    enum State {
        Ungreeted,
        Greeted,
    };

    explicit DSConvoServerConnection(QTcpSocket *socket, QObject *parent = nullptr);
    inline State state() const { return state_; }
    inline const QString &username() const { return username_; }
    void sendMessageBroadcast(const DSConvo::Protocol::MessageBroadcast &m);

signals:
    void helloRequested(QString &u, DSConvoServerConnection::HelloReplyError *r);
    void helloAccepted();
    void messageAccepted(const QString &m);
    void farewellSent();

private:
    void handleHelloRequest(const google::protobuf::Message*);
    void handleMessageRequest(const google::protobuf::Message*);
    void handleGoodbye();
    void sendHelloReply(DSConvoProtocol::HelloReplyPayload::HelloReplyError result,
                        const QString &username);

    State state_;
    QString username_;

protected slots:
    void streamMessageParsed(DSConvoStream::ParsedMessage&) override;

};

#endif
