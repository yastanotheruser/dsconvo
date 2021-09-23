#include "dsconvoclient.h"
#include "dsconvocommon.h"

using DSConvo::SocketErrorInfo;

DSConvoClient::DSConvoClient(QObject *parent)
    : QObject(parent)
    , socket_(nullptr)
    , clientConn(nullptr)
    , address_(QHostAddress::LocalHost)
    , port_(DSConvo::DEFAULT_PORT)
{
    setState(Inactive);
}

void DSConvoClient::clientConnect()
{
    qDebug("[DEBUG] DSConvoClient::clientConnect()");
    Q_ASSERT(socket_ == nullptr);
    setState(Connecting);
    socket_ = new QTcpSocket(this);
    socket_->connectToHost(address_, port_);
    connect(socket_, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket_, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(socketError(QAbstractSocket::SocketError)));
}

void DSConvoClient::clientDisconnect()
{
    qDebug("[DEBUG] DSConvoClient::clientDisconnect()");
    Q_ASSERT(socket_ != nullptr);
    setState(Disconnecting);
    socket_->disconnectFromHost();
}

void DSConvoClient::clearError()
{
    if (state_ == Error) {
        setState(Inactive);
    }
}

QString DSConvoClient::stateString()
{
    QString res;
    QTextStream ts(&res);

    switch (state_) {
    case Inactive:
        ts << tr("Inactivo");
        break;
    case Error: {
        auto errorInfo = qvariant_cast<SocketErrorInfo>(stateData_);
        ts << tr("Error: ") << errorInfo.second;
        break;
    }
    case Connecting:
        ts << tr("Conectando");
        break;
    case Connected:
        ts << tr("Conectado");
        break;
    case Disconnecting:
        ts << tr("Desconectando");
        break;
    }

    return res;
}

void DSConvoClient::setState(DSConvoClient::State state, const QVariant &data)
{
    state_ = state;
    stateData_ = data;
    emit stateChanged();
}

void DSConvoClient::socketConnected()
{
    clientConn = new DSConvoClientConnection(socket_, this);
    setState(Connected);
}

void DSConvoClient::socketDisconnected()
{
    clientConn->deleteLater();
    clientConn = nullptr;
    socket_ = nullptr;
    setState(Inactive);
}

void DSConvoClient::socketError(QAbstractSocket::SocketError error)
{
    QString errorString;
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        errorString = tr("La conexión fue rechazada");
        socket_->deleteLater();
        socket_ = nullptr;
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorString = tr("El servidor cerró la conexión");
        break;
    default:
        errorString = socket_->errorString();
        break;
    }

    SocketErrorInfo errorInfo(error, errorString);
    setState(Error, QVariant::fromValue(errorInfo));
}
