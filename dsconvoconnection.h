#ifndef DSCONVOCONNECTION_H
#define DSCONVOCONNECTION_H

#include <QObject>
#include <QTcpSocket>

class DSConvoConnection : public QObject
{
    Q_OBJECT

public:
    explicit DSConvoConnection(QTcpSocket *socket, QObject *parent = nullptr);
    ~DSConvoConnection();
    const QTcpSocket *socket() const;
    void close();

signals:
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void disconnected();

private:
    enum State {
        LENGTH,
        PAYLOAD,
    };

    static constexpr size_t BUFFER_SIZE = 1024;
    quint64 send(const QByteArray &data);
    QTcpSocket *socket_;
    quint8 *const buffer;
    State state;

private slots:
    void socketReadyRead();
};

#endif // DSCONVOCONNECTION_H
