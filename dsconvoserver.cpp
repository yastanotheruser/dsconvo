#include "dsconvoserver.h"
#include <QtDebug>
#include <QTextStream>
#include "dsconvocommon.h"
#include "dsconvoconnection.h"

using DSConvo::SocketErrorInfo;

DSConvoServer::DSConvoServer(const QHostAddress &address, quint16 port,
                             QObject *parent)
    : QObject(parent)
    , server(new QTcpServer)
    , address_(address)
    , port_(port)
{
    setStatus(Inactive);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

DSConvoServer::~DSConvoServer()
{
    if (server != nullptr) {
        delete server;
    }
}

bool DSConvoServer::listen()
{
    qDebug("[DEBUG] DSConvoServer::listen()");
    setStatus(Waiting);
    bool ok = server->listen(address_, port_);

    if (!ok) {
        SocketErrorInfo errorInfo(server->serverError(), server->errorString());
        setStatus(Error, QVariant::fromValue(errorInfo));
    } else {
        setStatus(Listening);
    }

    return ok;
}

void DSConvoServer::close()
{
    qDebug("[DEBUG] DSConvoServer::close()");
    setStatus(Closing);
    server->close();

    for (auto *c : clients) {
        c->deleteLater();
    }

    setStatus(Inactive);
}

void DSConvoServer::clearError()
{
    if (status_ == Error) {
        setStatus(Inactive);
    }
}

QString DSConvoServer::statusString()
{
    QString addressString;
    QString result;
    QTextStream ts(&result);
    QTextStream(&addressString) << address_.toString() << ":" << port_;

    switch (status_) {
    case Inactive:
        ts << tr("Inactivo");
        break;
    case Waiting:
        ts << tr("Iniciando servidor en ") << addressString;
        break;
    case Error: {
        auto errorInfo = qvariant_cast<SocketErrorInfo>(statusData_);
        ts << tr("Error: ") << errorInfo.second;
        break;
    }
    case Listening:
        ts << tr("Escuchando en ") << addressString;
        break;
    case Connection:
        ts << tr("Conexión desde ") << statusData_.toString();
        break;
    case Disconnection:
        ts << tr("Desconexión con ") << statusData_.toString();
        break;
    case Send:
        ts << tr("Enviados ") << statusData_.toUInt() << tr(" bytes");
        break;
    case Recv:
        ts << tr("Recibidos ") << statusData_.toUInt() << tr(" bytes");
        break;
    case Closing:
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

    DSConvoConnection *conn = new DSConvoConnection(socket, this);
    connect(conn, SIGNAL(dataSent(const QByteArray&)), this,
            SLOT(clientDataSent(const QByteArray&)));
    connect(conn, SIGNAL(dataReceived(const QByteArray&)), this,
            SLOT(clientDataReceived(const QByteArray&)));
    connect(conn, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    clients.insert(conn);
    setStatus(Connection, socket->peerAddress().toString());
}

void DSConvoServer::clientDataSent(const QByteArray &data)
{
    setStatus(Send, data.size());
}

void DSConvoServer::clientDataReceived(const QByteArray &data)
{
    setStatus(Recv, data.size());
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

    setStatus(Disconnection, conn->socket()->peerAddress().toString());
}
