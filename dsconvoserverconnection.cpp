#include "dsconvoserverconnection.h"
#include <QScopedPointer>
#include "dsconvocommon.h"

DSConvoServerConnection::DSConvoServerConnection(QTcpSocket *socket,
                                                 QObject *parent)
    : DSConvoConnection(socket, parent)
    , state_(Ungreeted)
{
}

void DSConvoServerConnection::sendMessageBroadcast(
        const DSConvo::Protocol::MessageBroadcast &m)
{
    using DSConvo::Protocol::makeMessageBroadcast;

    if (state_ != Greeted) {
        return;
    }

    send(makeMessageBroadcast(m.username.toStdString(), m.displayName.toStdString(),
                              m.message.toStdString()));
}

void DSConvoServerConnection::handleHelloRequest(const google::protobuf::Message *p)
{
    using DSConvoProtocol::HelloRequestPayload;
    using DSConvoProtocol::HelloReplyPayload;

    if (state_ != Ungreeted) {
        return;
    }

    const auto *payload = dynamic_cast<const HelloRequestPayload*>(p);
    if (payload == nullptr) {
        return;
    }

    QString username = QString::fromStdString(payload->username());
    username = DSConvo::normalizeText(username);

    // Ask helloRequested receivers
    auto result = HelloReplyPayload::OK;
    emit helloRequested(username, &result);

    if (username.isEmpty()) {
        result = HelloReplyPayload::BAD_USERNAME;
    }

    if (result == HelloReplyPayload::OK) {
        username_ = username;
    }

    sendHelloReply(result, username);
}

void DSConvoServerConnection::handleMessageRequest(const google::protobuf::Message *p)
{
    using DSConvoProtocol::MessageRequestPayload;

    if (state_ != Greeted) {
        return;
    }

    const auto *payload = dynamic_cast<const MessageRequestPayload*>(p);
    if (payload == nullptr) {
        return;
    }

    QString msg = DSConvo::normalizeText(QString::fromStdString(payload->msg()));
    if (msg.isEmpty() || msg.length() > DSConvo::MAX_MESSAGE) {
        return;
    }

    emit messageAccepted(msg);
}

void DSConvoServerConnection::handleGoodbye()
{
    if (state_ != Greeted) {
        return;
    }

    state_ = Ungreeted;
    emit farewellSent();
}

void DSConvoServerConnection::sendHelloReply(
        DSConvoProtocol::HelloReplyPayload::HelloReplyError result,
        const QString &username)
{
    using DSConvo::Protocol::makeHelloReply;

    if (state_ != Ungreeted) {
        return;
    }

    send(makeHelloReply(result, username.toStdString()));
    if (result == DSConvoProtocol::HelloReplyPayload::OK) {
        state_ = Greeted;
        emit helloAccepted();
    }
}

void DSConvoServerConnection::streamMessageParsed(
        DSConvoStream::ParsedMessage &message)
{
    qDebug("[DEBUG] DSConvoServerConnection::streamMessageParsed()");
    using DSConvoProtocol::DSConvoMessage;

    auto type = message.first;
    QScopedPointer<google::protobuf::Message> untypedPayload(message.second);

    switch (type) {
    case DSConvoMessage::HELLO_REQUEST:
        handleHelloRequest(untypedPayload.get());
        break;
    case DSConvoMessage::MESSAGE_REQUEST:
        handleMessageRequest(untypedPayload.get());
        break;
    case DSConvoMessage::GOODBYE:
        handleGoodbye();
        break;
    default:
        qDebug("[DEBUG] [DSConvoServerConnection::streamMessageParsed] "
               "server received bad message");
        return;
    }
}
