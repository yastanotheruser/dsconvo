#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QtDebug>
#include <QAbstractSocket>
#include "dsconvocommon.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
    , errorMessage(new QMessageBox(this))
    , client(new DSConvoClient)
{
    ui->setupUi(this);
    ui->addressLineEdit->setText(DSConvo::addressPortToString(DSConvo::serverAddress));
    ui->statusbar->showMessage(tr("Inactivo"));
    switchUi(CanConnect);
    errorMessage->setIcon(QMessageBox::Critical);
    errorMessage->setWindowTitle("Error");
    errorMessage->setStandardButtons(QMessageBox::Close);

    if (DSConvo::cmdline.address != nullptr) {
        ui->nameLineEdit->setFocus(Qt::OtherFocusReason);
    }

    connect(ui->connectPushButton, SIGNAL(clicked()), this,
            SLOT(toggleConnection()));
    connect(client, SIGNAL(statusChanged()), this, SLOT(clientStatusChanged()));
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

void ClientWindow::switchUi(ClientWindow::UiMode mode)
{
    switch (mode) {
    case CanConnect:
        ui->connectPushButton->setText(tr("Conectar"));
        ui->addressLineEdit->setDisabled(false);
        ui->connectPushButton->setDisabled(false);
        ui->nameLineEdit->setDisabled(false);
        ui->sendTextEdit->setDisabled(true);
        ui->sendPushButton->setDisabled(true);
        break;
    case CanSend:
        ui->connectPushButton->setText(tr("Desconectar"));
        ui->addressLineEdit->setDisabled(true);
        ui->connectPushButton->setDisabled(false);
        ui->nameLineEdit->setDisabled(true);
        ui->sendTextEdit->setDisabled(false);
        ui->sendPushButton->setDisabled(false);
        break;
    case Connecting:
    case Disconnecting:
        ui->addressLineEdit->setDisabled(true);
        ui->connectPushButton->setDisabled(true);
        ui->nameLineEdit->setDisabled(true);
        ui->sendTextEdit->setDisabled(true);
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

void ClientWindow::toggleConnection()
{
    if (client->socket() == nullptr) {
        doConnect();
    } else {
        doDisconnect();
    }
}

void ClientWindow::clientStatusChanged()
{
    using Status = DSConvoClient::Status;
    using DSConvo::SocketErrorInfo;

    Status status = client->status();
    switch (status) {
    case Status::Inactive:
        Q_ASSERT(uiMode != CanConnect);
        switchUi(CanConnect);
        break;
    case Status::Error: {
        QString errorString = qvariant_cast<SocketErrorInfo>(client->statusData()).second;
        errorMessage->setText(errorString);
        errorMessage->exec();
        break;
    }
    case Status::Connected:
        Q_ASSERT(uiMode == Connecting);
        switchUi(CanSend);
        break;
    default:
        // do nothing
        break;
    }

    ui->statusbar->showMessage(client->statusString());
}
