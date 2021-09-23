#ifndef DSCONVOPROTOCOL_H
#define DSCONVOPROTOCOL_H

#include <QPair>
#include <QByteArray>
#include <QString>
#include <istream>
#include "protobuf/dsconvo.pb.h"

namespace DSConvo {

namespace Protocol {

typedef QPair<DSConvoProtocol::DSConvoMessage::MessageType,
              google::protobuf::Message*> ParsedMessage;

struct MessageBroadcast {
    QString username;
    QString displayName;
    QString message;
};

auto const INVALID_PAYLOAD = reinterpret_cast<google::protobuf::Message*>(-1);

bool parseMessage(std::istream*, ParsedMessage&);

QByteArray makeHelloRequest(const std::string&);
QByteArray makeHelloReply(DSConvoProtocol::HelloReplyPayload::HelloReplyError,
                          const std::string& = std::string());
QByteArray makeMessageRequest(const std::string&);
QByteArray makeMessageBroadcast(const std::string&, const std::string&,
                                const std::string&);
QByteArray makeGoodbye();

} // namespace DSConvo::Protocol

QString formatMessage(const Protocol::MessageBroadcast &m);

} // namespace DSConvo

#endif
