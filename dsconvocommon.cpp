#include "dsconvocommon.h"
#include <QRegularExpression>
#include <QCommandLineParser>

#define PARSE_PAYLOAD(klass, from, into, fallback) \
    do { \
        klass *payload = new klass(); \
        if (payload->ParseFromString((from))) { \
            (into) = payload; \
        } else { \
            (into) = (fallback); \
        } \
    } while (0);

Q_DECLARE_METATYPE(::DSConvo::SocketErrorInfo);

namespace {

using DSConvoMessage = DSConvoProtocol::DSConvoMessage;

void parseCommandLine(const QApplication&);
void validateCommandLine();
inline QByteArray makeMessage(DSConvoMessage::MessageType,
                              const google::protobuf::Message&);

}

namespace DSConvo {

CommandLineOptions cmdline;
AddressPort serverAddress(QHostAddress::AnyIPv4, DEFAULT_PORT);

void initialize(QApplication &app)
{
    QApplication::setApplicationName("dsconvo");
    QApplication::setApplicationDisplayName("DSConvo chat");
    QApplication::setApplicationVersion("0.1");
    parseCommandLine(app);
}

bool validateAddressPort(const QString &addrPortString,
                         AddressPort &addrPort,
                         bool targetServerAddress)
{
    QRegularExpression addrPortPattern("^\\s*(\\S*?)(?::(\\d{1,5}))?\\s*$");
    QRegularExpressionMatch match = addrPortPattern.match(addrPortString);

    if (!match.hasMatch()) {
        return false;
    }

    QString addressString = match.captured(1);
    QString portString = match.captured(2);
    QHostAddress address;
    quint16 port;

    if (targetServerAddress && addressString.isEmpty()) {
        return false;
    }

    if (!portString.isEmpty()) {
        bool ok;
        quint32 port32 = portString.toUInt(&ok, 10);

        if (!ok || port32 == 0 || port32 > 65535) {
            return false;
        }

        port = port32 & 0xffff;
    } else {
        port = DSConvo::DEFAULT_PORT;
    }

    if (!addressString.isEmpty()) {
        address = QHostAddress(addressString);
        if (address.isNull()) {
            return false;
        }
    } else {
        address = QHostAddress::AnyIPv4;
    }

    addrPort.first = address;
    addrPort.second = port;
    return true;
}

QString addressPortToString(const AddressPort &addrPort)
{
    QString res;
    QTextStream(&res) << addrPort.first.toString() << ":"
                      << QString::number(addrPort.second);
    return res;
}

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
    const google::protobuf::Message *ptr = nullptr;

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
                          const std::string &username, const std::string &arg)
{
    DSConvoProtocol::HelloReplyPayload payload;
    payload.set_result(result);
    payload.set_username(username);
    payload.set_arg(arg);
    return makeMessage(DSConvoMessage::HELLO_REPLY, payload);
}

QByteArray makeMessageRequest(uint32_t seq, const std::string &msg)
{
    DSConvoProtocol::MessageRequestPayload payload;
    payload.set_seq(seq);
    payload.set_msg(msg);
    return makeMessage(DSConvoMessage::MESSAGE_REQUEST, payload);
}

QByteArray makeMessageBroadcast(const std::string &msg, const std::string &username)
{
    DSConvoProtocol::MessageBroadcastPayload payload;
    payload.set_msg(msg);
    payload.set_username(username);
    return makeMessage(DSConvoMessage::MESSAGE_BROADCAST, payload);
}

QByteArray makeGoodbye();

}

}

namespace {

void parseCommandLine(const QApplication &app)
{
    using DSConvo::cmdline;

    QCommandLineParser parser;
    parser.setApplicationDescription("DSConvo chat");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("address", app.tr("Dirección de escucha o conexión"));

    QCommandLineOption serverOption(QStringList() << "l" << "server", app.tr("Modo servidor"));
    parser.addOption(serverOption);

    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        cmdline.address = new QString(args.at(0));
    }

    cmdline.server = parser.isSet(serverOption);
    validateCommandLine();
}

void validateCommandLine()
{
    using DSConvo::cmdline;
    using DSConvo::serverAddress;

    if (DSConvo::cmdline.address != nullptr) {
        if (!DSConvo::validateAddressPort(*cmdline.address, serverAddress,
                                          !cmdline.server))
        {
            qCritical("Dirección inválida");
            exit(EXIT_FAILURE);
        }
    } else {
        if (!cmdline.server) {
            serverAddress.first = QHostAddress::LocalHost;
        }
    }
}

inline QByteArray makeMessage(DSConvoMessage::MessageType type,
                              const google::protobuf::Message &payload)
{
    DSConvoMessage message;
    std::string payloadBytes;
    std::string rawMessage;
    payload.SerializeToString(&payloadBytes);
    message.set_type(type);
    message.set_payload(payloadBytes);
    message.SerializeToString(&rawMessage);
    return QByteArray::fromStdString(rawMessage);
}

}
