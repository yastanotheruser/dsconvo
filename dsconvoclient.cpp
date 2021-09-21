#include "dsconvoclient.h"
#include "dsconvocommon.h"

using DSConvo::SocketErrorInfo;

DSConvoClient::DSConvoClient(QObject *parent)
    : QObject(parent)
    , socket_(nullptr)
    , dsconn(nullptr)
    , address_(QHostAddress::LocalHost)
    , port_(DSConvo::DEFAULT_PORT)
{
    setStatus(Inactive);
}

void DSConvoClient::clientConnect()
{
    qDebug("[DEBUG] DSConvoClient::clientConnect()");
    Q_ASSERT(socket_ == nullptr);
    setStatus(Connecting);
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
    setStatus(Disconnecting);
    socket_->disconnectFromHost();
}

void DSConvoClient::clearError()
{
    if (status_ == Error) {
        setStatus(Inactive);
    }
}

QString DSConvoClient::statusString()
{
    QString res;
    QTextStream ts(&res);

    switch (status_) {
    case Inactive:
        ts << tr("Inactivo");
        break;
    case Error: {
        auto errorInfo = qvariant_cast<SocketErrorInfo>(statusData_);
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

void DSConvoClient::setStatus(DSConvoClient::Status status, const QVariant &data)
{
    status_ = status;
    statusData_ = data;
    emit statusChanged();
}

void DSConvoClient::socketConnected()
{
    dsconn = new DSConvoConnection(socket_, this);
    setStatus(Connected);
}

void DSConvoClient::socketDisconnected()
{
    dsconn->deleteLater();
    dsconn = nullptr;
    socket_ = nullptr;
    setStatus(Inactive);
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

    SocketErrorInfo errorInfo(socket_->error(), errorString);
    setStatus(Error, QVariant::fromValue(errorInfo));
}
