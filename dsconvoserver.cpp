#include "dsconvoserver.h"
#include <QtDebug>
#include <QTextStream>
#include <QRandomGenerator>
#include <QSqlQuery>
#include "dsconvocommon.h"
#include "dsconvodatabase.h"

using DSConvo::SocketErrorInfo;

constexpr const char DSConvoServer::SERVER_USERNAME[];

DSConvoServer::DSConvoServer(QObject *parent)
    : QObject(parent)
    , server(new QTcpServer)
{
    setState(Inactive);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    if (DSConvo::Database::databaseEnabled) {
        DSConvo::Database::database(db);
    }
}

DSConvoServer::~DSConvoServer()
{
    if (server != nullptr) {
        delete server;
    }
}

QString DSConvoServer::stateString() const
{
    QString addressString;
    QString result;
    QTextStream ts(&result);
    QTextStream(&addressString) << address_.toString() << ":" << port_;

    switch (state_) {
    case Inactive:
        ts << tr("Inactivo");
        break;
    case Waiting:
        ts << tr("Iniciando servidor en ") << addressString;
        break;
    case Error: {
        auto errorInfo = qvariant_cast<SocketErrorInfo>(stateData_);
        ts << tr("Error: ") << errorInfo.second;
        break;
    }
    case Listening:
        ts << tr("Escuchando en ") << addressString;
        break;
    case Connection:
        ts << tr("Conexión desde ") << stateData_.toString();
        break;
    case Disconnection:
        ts << tr("Desconexión con ") << stateData_.toString();
        break;
    case Send:
        ts << tr("Enviados ") << stateData_.toUInt() << tr(" bytes");
        break;
    case Recv:
        ts << tr("Recibidos ") << stateData_.toUInt() << tr(" bytes");
        break;
    case Closing:
        ts << tr("Terminando servidor");
        break;
    default:
        break;
    }

    return result;
}

bool DSConvoServer::listen()
{
    qDebug("[DEBUG] DSConvoServer::listen()");
    setState(Waiting);
    bool ok = server->listen(address_, port_);

    if (ok) {
        addUser(SERVER_USERNAME);
        setState(Listening);
    } else {
        SocketErrorInfo errorInfo(server->serverError(), server->errorString());
        setState(Error, QVariant::fromValue(errorInfo));
    }

    return ok;
}

void DSConvoServer::close()
{
    qDebug("[DEBUG] DSConvoServer::close()");
    setState(Closing);
    server->close();

    for (auto *c : clients) {
        c->deleteLater();
    }

    clients.clear();
    users.clear();
    displayNames.clear();
    setState(Inactive);
}

void DSConvoServer::clearError()
{
    if (state_ == Error) {
        setState(Inactive);
    }
}

void DSConvoServer::broadcastMessage(const QString &message)
{
    DSConvo::Protocol::MessageBroadcast m;
    m.username = SERVER_USERNAME;
    m.displayName = displayNames[SERVER_USERNAME];
    m.message = message;

    for (auto *c : clients) {
        c->sendMessageBroadcast(m);
    }

    emit messaged(m);
}

void DSConvoServer::setState(DSConvoServer::State state,
                              const QVariant &data)
{
    state_ = state;
    stateData_ = data;
    emit stateChanged();
}

QString DSConvoServer::makeDisplayName(const QString &username) const
{
    QChar typeChar = username != SERVER_USERNAME ? 'A' : 'S';
    QString name;

    if (typeChar != 'S' && db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT displayName FROM Users WHERE username = ?");
        query.addBindValue(username);

        if (query.exec() && query.next()) {
            QVariant v = query.value(0);
            typeChar = 'U';

            if (!v.isNull()) {
                name = v.toString();
            }
        }
    }

    QString displayName;
    QTextStream ts(&displayName);
    ts << "[" << typeChar << "] " << username;

    if (!name.isEmpty()) {
        ts << " (" << name << ")";
    }

    return displayName;
}

void DSConvoServer::newConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    if (socket == nullptr) {
        return;
    }

    DSConvoServerConnection *conn = new DSConvoServerConnection(socket, this);
    connect(conn, SIGNAL(dataSent(const QByteArray&)), this,
            SLOT(clientDataSent(const QByteArray&)));
    connect(conn, SIGNAL(dataReceived(const QByteArray&)), this,
            SLOT(clientDataReceived(const QByteArray&)));
    connect(conn, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(conn, SIGNAL(helloRequested(QString&, DSConvoServerConnection::HelloReplyError*)),
            this, SLOT(clientHelloRequested(QString&, DSConvoServerConnection::HelloReplyError*)));
    connect(conn, SIGNAL(helloAccepted()), this, SLOT(clientHelloAccepted()));
    connect(conn, SIGNAL(messageAccepted(const QString&)),
            this, SLOT(clientMessageAccepted(const QString&)));
    connect(conn, SIGNAL(farewellSent()), this, SLOT(clientFarewellSent()));
    clients.insert(conn);
    setState(Connection, socket->peerAddress().toString());
}

void DSConvoServer::clientDataSent(const QByteArray &data)
{
    setState(Send, data.size());
}

void DSConvoServer::clientDataReceived(const QByteArray &data)
{
    setState(Recv, data.size());
}

void DSConvoServer::clientDisconnected()
{
    QObject *s = sender();
    if (!clients.contains(static_cast<DSConvoServerConnection*>(s))) {
        return;
    }

    auto *conn = qobject_cast<DSConvoServerConnection*>(s);
    Q_ASSERT(conn != nullptr);
    qDebug("[DEBUG] DSConvoServer::clientDisconnected()");

    if (conn->state() == DSConvoServerConnection::Greeted) {
        removeUser(conn->username());
    }

    clients.remove(conn);
    conn->deleteLater();
    setState(Disconnection, conn->socket()->peerAddress().toString());
}

void DSConvoServer::clientHelloRequested(QString &username,
                                         DSConvoServerConnection::HelloReplyError *res)
{
    using DSConvoProtocol::HelloReplyPayload;

    if (*res != HelloReplyPayload::OK) {
        return;
    }

    if (username.isEmpty()) {
        quint32 v = QRandomGenerator::global()->generate() % 100000000UL;
        QTextStream(&username) << "user"
                               << qSetFieldWidth(8) << qSetPadChar('0')
                               << v;
    }

    if (users.contains(username)) {
        *res = HelloReplyPayload::USERNAME_IN_USE;
    }
}

void DSConvoServer::clientHelloAccepted()
{
    if (auto *client = qobject_cast<DSConvoServerConnection*>(sender())) {
        addUser(client->username());
    } else {
        qDebug("[DEBUG] [DSConvoServer::clientHelloAccepted] bad cast");
    }
}

void DSConvoServer::clientMessageAccepted(const QString &message)
{
    auto *client = qobject_cast<DSConvoServerConnection*>(sender());
    if (client == nullptr) {
        qDebug("[DEBUG] [DSConvoServer::clientMessageAccepted] bad cast");
        return;
    }

    const QString &username = client->username();
    Q_ASSERT(displayNames.contains(username));
    const QString &displayName = displayNames[username];
    DSConvo::Protocol::MessageBroadcast m = {username, displayName, message};

    for (auto *c : clients) {
        c->sendMessageBroadcast(m);
    }

    emit messaged(m);
}

void DSConvoServer::clientFarewellSent()
{
    if (auto *client = qobject_cast<DSConvoServerConnection*>(sender())) {
        removeUser(client->username());
    } else {
        qDebug("[DEBUG] [DSConvoServer::clientFarewellSent] bad cast");
    }
}
