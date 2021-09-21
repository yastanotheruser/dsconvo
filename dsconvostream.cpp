#include "dsconvostream.h"

#define IO_OPENMODE (std::ios_base::binary | \
    std::ios_base::in | \
    std::ios_base::out)

DSConvoStream::DSConvoStream(QObject *parent)
    : QObject(parent)
    , buffer(new QByteArray)
    , bufferOffset(0)
    , chunkOffset(0)
    , state(Length)
    , io(IO_OPENMODE)
{
    buffer->resize(BUFFER_SIZE);
}

void DSConvoStream::handleRecvData(const QByteArray &data)
{
    static constexpr int LENGTH_CHUNK_SIZE = sizeof(quint16);
    int dataOffset = 0;
    int dataSize = data.size();

    while (dataOffset < dataSize) {
        int chunkSize;
        switch (state) {
        case Length:
            chunkSize = LENGTH_CHUNK_SIZE;
            break;
        case Message:
            chunkSize = stateArg;
            break;
        }

        int numBytes = qMin(dataSize - dataOffset, chunkSize - chunkOffset);
        numBytes = qMin(numBytes, BUFFER_SIZE - bufferOffset);
        buffer->replace(bufferOffset, numBytes, data.mid(dataOffset, numBytes));
        dataOffset += numBytes;
        chunkOffset += numBytes;
        bufferOffset += numBytes;

        if (chunkOffset == chunkSize || bufferOffset == BUFFER_SIZE) {
            handleBufferData(chunkSize);
        }
    }
}

void DSConvoStream::handleBufferData(int chunkSize)
{
    if (chunkSize < BUFFER_SIZE) {
        Q_ASSERT(bufferOffset == chunkSize && chunkOffset == chunkSize);
    }

    switch (state) {
    case Length:
        stateArg = *reinterpret_cast<const quint32*>(buffer->constData()) & 0xffff;
        qDebug("[DEBUG] DSConvoStream::handleBufferData(%d) "
               "(state = Length ; stateArg = %d)", chunkSize, stateArg);

        if (stateArg == 0) {
            qDebug("[DEBUG] [DSConvoStream Length] ignoring empty message");
            return;
        }

        state = Message;
        io.str(std::string());
        io.clear();
        break;
    case Message:
        qDebug("[DEBUG] DSConvoStream::handleBufferData(%d) "
               "(state = Message ; chunkOffset = %d)", chunkSize, chunkOffset);
        io.write(buffer->constData(), bufferOffset);

        if (chunkOffset == chunkSize) {
            qDebug("[DEBUG] [DSConvoStream Message] message received");
            handleMessage();
        }

        break;
    }

    if (chunkOffset == chunkSize) {
        bufferOffset = 0;
        chunkOffset = 0;
    }
}

bool DSConvoStream::handleMessage()
{
    using namespace DSConvo::Protocol;

    ParsedMessage message;
    if (!parseMessage(&io, message)) {
        return false;
    }

    emit messageParsed(message);
    return true;
}
