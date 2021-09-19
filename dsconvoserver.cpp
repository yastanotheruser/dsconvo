#include "dsconvoserver.h"
#include <QtDebug>
#include <QTextStream>
#include "dsconvoconnection.h"

DSConvoServer::DSConvoServer(const QHostAddress &address, quint16 port,
                             QObject *parent)
    : QObject(parent)
    , server(new QTcpServer)
    , address_(address)
    , port_(port)
{
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    setStatus(Status::Inactive);
}

DSConvoServer::~DSConvoServer()
{
    if (server != nullptr) {
        delete server;
    }
}

const QHostAddress &DSConvoServer::address() const
{
    return address_;
}

quint16 DSConvoServer::port() const
{
    return port_;
}

bool DSConvoServer::listening() const
{
    return server->isListening();
}

DSConvoServer::Status DSConvoServer::status() const
{
    return status_;
}

const QVariant &DSConvoServer::statusData() const
{
    return statusData_;
}

bool DSConvoServer::listen()
{
    qDebug("[DEBUG] DSConvoServer::listen()");
    setStatus(Status::Waiting);
    bool ok = server->listen(address_, port_);

    if (!ok) {
        SocketErrorInfo errorInfo(server->serverError(), server->errorString());
        setStatus(Status::Error, QVariant::fromValue(errorInfo));
    } else {
        setStatus(Status::Listening);
    }

    return ok;
}

void DSConvoServer::close()
{
    qDebug("[DEBUG] DSConvoServer::close()");
    setStatus(Status::Closing);
    server->close();

    for (auto *c : clients) {
        c->deleteLater();
    }

    setStatus(Status::Inactive);
}

void DSConvoServer::clearError()
{
    if (status_ == Status::Error) {
        setStatus(Status::Inactive);
    }
}

QString DSConvoServer::statusString()
{
    QString addressString;
    QString result;
    QTextStream ts(&result);
    QTextStream(&addressString) << address_.toString() << ":" << port_;

    switch (status_) {
    case Status::Inactive:
        ts << tr("Inactivo");
        break;
    case Status::Waiting:
        ts << tr("Iniciando servidor en ") << addressString;
        break;
    case Status::Error: {
        SocketErrorInfo errorInfo = qvariant_cast<SocketErrorInfo>(statusData_);
        ts << tr("Error: ") << errorInfo.second;
        break;
    }
    case Status::Listening:
        ts << tr("Escuchando en ") << addressString;
        break;
    case Status::Connection:
        ts << tr("Conexión desde ") << statusData_.toString();
        break;
    case Status::Disconnection:
        ts << tr("Desconexión con ") << statusData_.toString();
        break;
    case Status::Send:
        ts << tr("Enviados ") << statusData_.toUInt() << tr(" bytes");
        break;
    case Status::Recv:
        ts << tr("Recibidos ") << statusData_.toUInt() << tr(" bytes");
        break;
    case Status::Closing:
        ts << tr("Terminando servidor");
        break;
    default:
        break;
    }

    return result;
}

void DSConvoServer::setStatus(DSConvoServer::Status status,
                              const QVariant &data)
{
    status_ = status;
    statusData_ = data;
    emit statusChanged();
}

void DSConvoServer::newConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    if (socket == nullptr) {
        return;
    }

    DSConvoConnection *conn = new DSConvoConnection(socket);
    connect(conn, SIGNAL(dataSent(const QByteArray&)), this,
            SLOT(clientDataSent(const QByteArray&)));
    connect(conn, SIGNAL(dataReceived(const QByteArray&)), this,
            SLOT(clientDataReceived(const QByteArray&)));
    connect(conn, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    clients.insert(conn);
    setStatus(Status::Connection, socket->peerAddress().toString());
}

void DSConvoServer::clientDataSent(const QByteArray &data)
{
    setStatus(Status::Send, data.size());
}

void DSConvoServer::clientDataReceived(const QByteArray &data)
{
    setStatus(Status::Recv, data.size());
}

void DSConvoServer::clientDisconnected()
{
    DSConvoConnection *conn = qobject_cast<DSConvoConnection*>(sender());
    qDebug("[DEBUG] DSConvoServer::clientDisconnected() (conn->socket_ = %p)",
          conn->socket());

    if (conn != nullptr) {
        clients.remove(conn);
        conn->deleteLater();
    }

    setStatus(Status::Disconnection, conn->socket()->peerAddress().toString());
}
