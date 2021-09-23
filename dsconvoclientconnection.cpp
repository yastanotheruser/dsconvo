#include "dsconvoclientconnection.h"
#include <QtDebug>
#include <QScopedPointer>
#include "dsconvocommon.h"

DSConvoClientConnection::DSConvoClientConnection(QTcpSocket *socket,
                                                 QObject *parent)
    : DSConvoConnection(socket, parent)
    , state_(Ungreeted)
{
}

void DSConvoClientConnection::sendHello(const QString &username)
{
    Q_ASSERT(state_ == Ungreeted);
    qDebug("[DEBUG] DSConvoClientConnection::sendHello(%s)",
           username.toLocal8Bit().constData());
    state_ = Greeting;
    send(DSConvo::Protocol::makeHelloRequest(username.toStdString()));
}

void DSConvoClientConnection::sendMessage(const QString &message)
{
    Q_ASSERT(state_ == Greeted);
    send(DSConvo::Protocol::makeMessageRequest(message.toStdString()));
}

void DSConvoClientConnection::sendGoodbye()
{
    send(DSConvo::Protocol::makeGoodbye());
    state_ = Ungreeted;
}

void DSConvoClientConnection::handleHelloReply(const google::protobuf::Message *p)
{
    using DSConvoProtocol::HelloReplyPayload;

    if (state_ != Greeting) {
        return;
    }

    const auto *payload = dynamic_cast<const HelloReplyPayload*>(p);
    if (payload == nullptr) {
        return;
    }

    auto result = payload->result();
    switch (result) {
    case HelloReplyPayload::OK:
        username_ = QString::fromStdString(payload->username());
        state_ = Greeted;
        emit greeted();
        break;
    case HelloReplyPayload::BAD_USERNAME:
        emit errorOccurred(BadUsername);
        break;
    case HelloReplyPayload::USERNAME_IN_USE:
        emit errorOccurred(RepeatedUsername);
        break;
    case HelloReplyPayload::SERVER_LOAD:
        emit errorOccurred(ServerLoad);
        break;
    default:
        // not expected
        break;
    }

    if (state_ == Greeting) {
        state_ = Ungreeted;
    }
}

void DSConvoClientConnection::handleMessageBroadcast(const google::protobuf::Message *p)
{
    using DSConvoProtocol::MessageBroadcastPayload;

    if (state_ != Greeted) {
        return;
    }

    const auto *payload = dynamic_cast<const MessageBroadcastPayload*>(p);
    if (payload == nullptr) {
        return;
    }

    DSConvo::Protocol::MessageBroadcast m;
    m.username = QString::fromStdString(payload->username());
    m.displayName = QString::fromStdString(payload->display_name());
    m.message = QString::fromStdString(payload->msg());
    emit messaged(m);
}

void DSConvoClientConnection::streamMessageParsed(DSConvoStream::ParsedMessage &message)
{
    using DSConvoProtocol::DSConvoMessage;

    auto type = message.first;
    QScopedPointer<google::protobuf::Message> untypedPayload(message.second);

    switch (type) {
    case DSConvoMessage::HELLO_REPLY:
        handleHelloReply(untypedPayload.get());
        break;
    case DSConvoMessage::MESSAGE_BROADCAST:
        handleMessageBroadcast(untypedPayload.get());
        break;
    default:
        qDebug("[DEBUG] [DSConvoClientConnection::streamMessageParsed] "
               "client received bad message");
        return;
    }
}
