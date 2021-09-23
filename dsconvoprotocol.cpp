#include "dsconvoprotocol.h"
#include <QRegularExpression>
#include <QTextStream>

#define PARSE_PAYLOAD(klass, from, into, fallback) \
    do { \
        klass *payload = new klass(); \
        if (payload->ParseFromString((from))) { \
            (into) = payload; \
        } else { \
            (into) = (fallback); \
        } \
    } while (0);

namespace {

using DSConvoMessage = DSConvoProtocol::DSConvoMessage;
inline QByteArray makeMessage(DSConvoMessage::MessageType,
                              const google::protobuf::Message&);
inline QByteArray makeMessage(DSConvoMessage::MessageType);

} // anonymous

namespace DSConvo {

namespace Protocol {

bool parseMessage(std::istream *is, ParsedMessage &into)
{
    using namespace DSConvoProtocol;

    DSConvoMessage message;
    if (!message.ParseFromIstream(is)) {
        qDebug("[DEBUG] [DSConvo::Protocol::parseMessage()] bad message");
        return false;
    }

    DSConvoMessage::MessageType type = message.type();
    const std::string &data = message.payload();
    google::protobuf::Message *ptr = nullptr;

    switch (type) {
    case DSConvoMessage::HELLO_REQUEST:
        PARSE_PAYLOAD(HelloRequestPayload, data, ptr, INVALID_PAYLOAD);
        break;
    case DSConvoMessage::HELLO_REPLY:
        PARSE_PAYLOAD(HelloReplyPayload, data, ptr, INVALID_PAYLOAD);
        break;
    case DSConvoMessage::MESSAGE_REQUEST:
        PARSE_PAYLOAD(MessageRequestPayload, data, ptr, INVALID_PAYLOAD);
        break;
    case DSConvoMessage::MESSAGE_BROADCAST:
        PARSE_PAYLOAD(MessageBroadcastPayload, data, ptr, INVALID_PAYLOAD);
        break;
    default:
        // unreachable?
        return false;
    }

    if (ptr == INVALID_PAYLOAD) {
        return false;
    }

    into.first = type;
    into.second = ptr;
    return true;
}

QByteArray makeHelloRequest(const std::string &username)
{
    DSConvoProtocol::HelloRequestPayload payload;
    payload.set_username(username);
    return makeMessage(DSConvoMessage::HELLO_REQUEST, payload);
}

QByteArray makeHelloReply(DSConvoProtocol::HelloReplyPayload::HelloReplyError result,
                          const std::string &username)
{
    Q_ASSERT(result != DSConvoProtocol::HelloReplyPayload::INVALID);
    DSConvoProtocol::HelloReplyPayload payload;
    payload.set_result(result);
    payload.set_username(username);
    return makeMessage(DSConvoMessage::HELLO_REPLY, payload);
}

QByteArray makeMessageRequest(const std::string &msg)
{
    DSConvoProtocol::MessageRequestPayload payload;
    payload.set_msg(msg);
    return makeMessage(DSConvoMessage::MESSAGE_REQUEST, payload);
}

QByteArray makeMessageBroadcast(const std::string &username,
                                const std::string &displayName,
                                const std::string &msg)
{
    DSConvoProtocol::MessageBroadcastPayload payload;
    payload.set_username(username);
    payload.set_display_name(displayName);
    payload.set_msg(msg);
    return makeMessage(DSConvoMessage::MESSAGE_BROADCAST, payload);
}

QByteArray makeGoodbye()
{
    return makeMessage(DSConvoMessage::GOODBYE);
}

} // namespace DSConvo::Protocol

QString formatMessage(const Protocol::MessageBroadcast &m)
{
    static QRegularExpression const lineBreakGroupPattern("[\r\n]{2,}");
    QString text = m.message;
    text.replace(lineBreakGroupPattern, "\n");
    text.prepend("    ");
    text.replace("\n", "\n    ");

    QString out;
    QTextStream(&out) << m.displayName << " >\n" << text << "\n";
    return out;
}

} // namespace DSConvo

namespace {

inline QByteArray makeMessage(DSConvoMessage::MessageType type,
                              const google::protobuf::Message &payload)
{
    Q_ASSERT(type != DSConvoMessage::INVALID);
    DSConvoMessage message;
    std::string payloadBytes;
    std::string rawMessage;
    payload.SerializeToString(&payloadBytes);
    message.set_type(type);
    message.set_payload(payloadBytes);
    message.SerializeToString(&rawMessage);
    return QByteArray::fromStdString(rawMessage);
}

inline QByteArray makeMessage(DSConvoMessage::MessageType type)
{
    Q_ASSERT(type != DSConvoMessage::INVALID);
    DSConvoMessage message;
    std::string rawMessage;
    message.set_type(type);
    message.SerializeToString(&rawMessage);
    return QByteArray::fromStdString(rawMessage);
}

} // anonymous
