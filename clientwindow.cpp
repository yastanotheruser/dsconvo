#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QtDebug>
#include <QList>
#include <QAbstractSocket>
#include <QCompleter>
#include "dsconvocommon.h"
#include "dsconvodatabase.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
    , errorMessage(new QMessageBox(this))
    , client(new DSConvoClient(this))
    , completerTimer(new QTimer(this))
    , canUpdateCompleter(true)
{
    ui->setupUi(this);
    ui->addressLineEdit->setText(DSConvo::addressPortToString(DSConvo::serverAddress));
    ui->nameLineEdit->installEventFilter(this);
    ui->recvPlainTextEdit->setMaximumBlockCount(32);
    ui->statusbar->showMessage(tr("Inactivo"));
    new DSConvo::QtUtil::PlainTextEditLimit(DSConvo::MAX_MESSAGE,
                                            ui->sendPlainTextEdit);
    switchUi(CanConnect);
    errorMessage->setIcon(QMessageBox::Critical);
    errorMessage->setWindowTitle("Error");
    errorMessage->setStandardButtons(QMessageBox::Close);

    if (DSConvo::cmdline.address != nullptr) {
        ui->nameLineEdit->setFocus(Qt::OtherFocusReason);
    }

    connect(ui->connectPushButton, SIGNAL(clicked()), this, SLOT(toggleConnection()));
    connect(ui->sendPushButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(client, SIGNAL(stateChanged()), this, SLOT(clientStateChanged()));
    connect(completerTimer, SIGNAL(timeout()), this, SLOT(completerTimerTimeout()));
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

bool ClientWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->nameLineEdit) {
        if (event->type() == QEvent::FocusIn) {
            updateCompleter();
        }
    }

    return QObject::eventFilter(object, event);
}

void ClientWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        QCoreApplication::quit();
    }

    QWidget::keyPressEvent(event);
    return;
}

void ClientWindow::switchUi(ClientWindow::UiMode mode)
{
    switch (mode) {
    case CanConnect:
        ui->connectPushButton->setText(tr("Conectar"));
        ui->addressLineEdit->setDisabled(false);
        ui->connectPushButton->setDisabled(false);
        ui->nameLineEdit->setDisabled(false);
        ui->sendPlainTextEdit->setDisabled(true);
        ui->sendPushButton->setDisabled(true);
        break;
    case CanSend:
        ui->connectPushButton->setText(tr("Desconectar"));
        ui->addressLineEdit->setDisabled(true);
        ui->connectPushButton->setDisabled(false);
        ui->nameLineEdit->setDisabled(true);
        ui->sendPlainTextEdit->setDisabled(false);
        ui->sendPushButton->setDisabled(false);
        break;
    case Connecting:
    case Disconnecting:
        ui->addressLineEdit->setDisabled(true);
        ui->connectPushButton->setDisabled(true);
        ui->nameLineEdit->setDisabled(true);
        ui->sendPlainTextEdit->setDisabled(true);
        ui->sendPushButton->setDisabled(true);
        break;
    default:
        break;
    }

    uiMode = mode;
}

void ClientWindow::doConnect()
{
    using DSConvo::serverAddress;

    QString addrPortString = ui->addressLineEdit->text();
    if (!DSConvo::validateAddressPort(addrPortString, serverAddress, true)) {
        errorMessage->setText(tr("Dirección inválida"));
        errorMessage->exec();
        return;
    }

    ui->addressLineEdit->setText(DSConvo::addressPortToString(serverAddress));
    switchUi(Connecting);
    client->setAddress(serverAddress.first);
    client->setPort(serverAddress.second);
    client->clientConnect();
}

void ClientWindow::doDisconnect()
{
    switchUi(Disconnecting);
    client->clientDisconnect();
}

void ClientWindow::initClientConnection()
{
    DSConvoClientConnection *conn = client->clientConnection();
    Q_ASSERT(conn != nullptr);
    connect(conn, SIGNAL(greeted()), this, SLOT(clientConnGreeted()));
    connect(conn, SIGNAL(messaged(const DSConvo::Protocol::MessageBroadcast&)), this,
            SLOT(clientConnMessaged(const DSConvo::Protocol::MessageBroadcast&)));
    connect(conn, SIGNAL(errorOccurred(DSConvoClientConnection::Error)), this,
            SLOT(clientConnErrorOccurred(DSConvoClientConnection::Error)));
    conn->sendHello(DSConvo::normalizeText(ui->nameLineEdit->text()));
}

void ClientWindow::updateCompleter()
{
    using DSConvo::Database::queryUsers;
    using DSConvo::Database::User;

    if (!canUpdateCompleter) {
        return;
    }

    canUpdateCompleter = false;

    QStringList usernames;
    queryUsers([&usernames](const User &u) { usernames.append(u.username); });

    QCompleter *completer = new QCompleter(usernames, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->nameLineEdit->setCompleter(completer);
    completerTimer->start(5000);
    qDebug("[DEBUG] [ClientWindow::updateCompleter] completer updated");
}

void ClientWindow::toggleConnection()
{
    if (client->socket() == nullptr) {
        doConnect();
    } else {
        doDisconnect();
    }
}

void ClientWindow::sendMessage()
{
    QString message = DSConvo::normalizeText(ui->sendPlainTextEdit->toPlainText());
    if (message.isEmpty()) {
        return;
    }

    DSConvoClientConnection *conn = client->clientConnection();
    Q_ASSERT(conn != nullptr);
    conn->sendMessage(message);
    ui->sendPlainTextEdit->clear();
}

void ClientWindow::clientStateChanged()
{
    using State = DSConvoClient::State;
    using DSConvo::SocketErrorInfo;

    State state = client->state();
    switch (state) {
    case State::Inactive:
        ui->sendPlainTextEdit->clear();
        if (uiMode != CanConnect) {
            switchUi(CanConnect);
        }
        break;
    case State::Error: {
        QString errorString = qvariant_cast<SocketErrorInfo>(client->stateData()).second;
        errorMessage->setText(errorString);
        errorMessage->exec();
        client->clearError();
        return;
    }
    case State::Connected:
        initClientConnection();
        break;
    default:
        // do nothing
        break;
    }

    ui->statusbar->showMessage(client->stateString());
}

void ClientWindow::clientConnGreeted()
{
    DSConvoClientConnection *conn = client->clientConnection();
    Q_ASSERT(conn != nullptr);
    Q_ASSERT(uiMode == Connecting);
    switchUi(CanSend);
    ui->nameLineEdit->setText(conn->username());
    ui->recvPlainTextEdit->clear();
}

void ClientWindow::clientConnMessaged(const DSConvo::Protocol::MessageBroadcast &m)
{
    ui->recvPlainTextEdit->appendPlainText(DSConvo::formatMessage(m));
}

void ClientWindow::clientConnErrorOccurred(DSConvoClientConnection::Error error)
{
    bool shallDisconnect = true;
    QString errorString;

    switch (error) {
    case DSConvoClientConnection::BadUsername:
        errorString = "Nombre de usuario inválido";
        break;
    case DSConvoClientConnection::RepeatedUsername:
        errorString = "El nombre de usuario ya fue tomado";
        break;
    case DSConvoClientConnection::ServerLoad:
        errorString = "Demasiados usuarios conectados";
        break;
    }

    errorMessage->setText(errorString);
    errorMessage->exec();

    if (shallDisconnect) {
        doDisconnect();
    }
}

void ClientWindow::completerTimerTimeout()
{
    canUpdateCompleter = true;
}
