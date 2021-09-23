#ifndef DSCONVOSTREAM_H
#define DSCONVOSTREAM_H

#include <QObject>
#include <sstream>
#include "dsconvoprotocol.h"
#include "protobuf/dsconvo.pb.h"

class DSConvoStream : public QObject
{
    Q_OBJECT

public:
    using MessageType = DSConvoProtocol::DSConvoMessage::MessageType;
    using ParsedMessage = DSConvo::Protocol::ParsedMessage;
    explicit DSConvoStream(QObject *parent = nullptr);
    void handleRecvData(const QByteArray &data);

signals:
    void messageParsed(DSConvoStream::ParsedMessage&);

private:
    enum State {
        Length,
        Message,
    };

    void handleBufferData(int chunkSize);
    bool handleMessage();

    static constexpr int BUFFER_SIZE = 1024;
    QByteArray *const buffer;
    int bufferOffset;
    int chunkOffset;
    State state;
    quint32 stateArg;
    std::stringstream io;

};

#endif
