#ifndef DSCONVOCOMMON_H
#define DSCONVOCOMMON_H

#include <QPair>
#include <QByteArray>
#include <QHostAddress>
#include <QApplication>
#include <istream>
#include "protobuf/dsconvo.pb.h"

namespace DSConvo {

typedef QPair<QHostAddress, quint16> AddressPort;
typedef QPair<QAbstractSocket::SocketError, QString> SocketErrorInfo;

struct CommandLineOptions {
    QString *address;
    bool server;
};

constexpr quint16 DEFAULT_PORT = 5500;
extern CommandLineOptions cmdline;
extern AddressPort serverAddress;

void initialize(QApplication &app);
bool validateAddressPort(const QString&, AddressPort&, bool = false);
QString addressPortToString(const AddressPort&);

namespace Protocol {

typedef QPair<DSConvoProtocol::DSConvoMessage::MessageType, const void*> ParsedMessage;

auto const INVALID_PAYLOAD = reinterpret_cast<const google::protobuf::Message*>(-1);

bool parseMessage(std::istream*, ParsedMessage&);

QByteArray makeHelloRequest(const std::string&);
QByteArray makeHelloReply(DSConvoProtocol::HelloReplyPayload::HelloReplyError,
                          const std::string&, const std::string& = std::string());
QByteArray makeMessageRequest(uint32_t, const std::string&);
QByteArray makeMessageBroadcast(const std::string&, const std::string&);
QByteArray makeGoodbye();

}

}
#endif
