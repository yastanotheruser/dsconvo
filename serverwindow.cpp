#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QtDebug>
#include <QAbstractSocket>
#include "dsconvocommon.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
    , errorMessage(new QMessageBox(this))
    , server(new DSConvoServer)
{
    const DSConvo::AddressPort &addr = DSConvo::serverAddress;
    ui->setupUi(this);
    ui->portLineEdit->setText(QString::number(addr.second));
    new DSConvo::QtUtil::PlainTextEditLimit(DSConvo::MAX_MESSAGE,
                                            ui->sendPlainTextEdit);
    errorMessage->setIcon(QMessageBox::Critical);
    errorMessage->setWindowTitle("Error");
    errorMessage->setStandardButtons(QMessageBox::Close);
    server->setAddress(addr.first);
    server->setPort(addr.second);

    connect(ui->listenPushButton, SIGNAL(clicked()), this, SLOT(toggleServer()));
    connect(ui->sendPushButton, SIGNAL(clicked()), this, SLOT(broadcast()));
    connect(server, SIGNAL(stateChanged()), this, SLOT(serverStateChanged()));
    connect(server, SIGNAL(messaged(const DSConvo::Protocol::MessageBroadcast&)),
            this, SLOT(serverMessaged(const DSConvo::Protocol::MessageBroadcast&)));
}

ServerWindow::~ServerWindow()
{
    delete ui;
    delete server;
}

void ServerWindow::toggleServer()
{
    if (!server->listening()) {
        QString portString = ui->portLineEdit->text();
        quint16 port;

        if (!DSConvo::validatePort(portString, &port)) {
            errorMessage->setText(tr("Puerto inválido"));
            errorMessage->exec();
            return;
        }

        server->setPort(port);
        ui->portLineEdit->setText(QString::number(port));
        ui->listenPushButton->setDisabled(true);

        if (server->listen()) {
            ui->portLineEdit->setDisabled(true);
            ui->sendPlainTextEdit->setDisabled(false);
            ui->sendPushButton->setDisabled(false);
            ui->listenPushButton->setText(tr("Detener"));
            ui->recvPlainTextEdit->clear();
        }

        ui->listenPushButton->setDisabled(false);
    } else {
        ui->listenPushButton->setDisabled(true);
        server->close();
        ui->portLineEdit->setDisabled(false);
        ui->listenPushButton->setDisabled(false);
        ui->sendPlainTextEdit->setDisabled(true);
        ui->sendPushButton->setDisabled(true);
        ui->listenPushButton->setText(tr("Iniciar servidor"));
        ui->sendPlainTextEdit->clear();
    }
}

void ServerWindow::broadcast()
{
    QString message = ui->sendPlainTextEdit->toPlainText();
    if (message.isEmpty()) {
        return;
    }

    server->broadcastMessage(DSConvo::normalizeText(message));
    ui->sendPlainTextEdit->clear();
}

void ServerWindow::serverStateChanged()
{
    if (server->state() != DSConvoServer::Error) {
        ui->statusbar->showMessage(server->stateString());
        return;
    }

    auto errorInfo = qvariant_cast<DSConvo::SocketErrorInfo>(server->stateData());
    QString errorString;

    switch (errorInfo.first) {
    case QAbstractSocket::SocketError::AddressInUseError:
        errorString = tr("El puerto ya está siendo usado");
        break;
    default:
        errorString = errorInfo.second;
        break;
    }

    errorMessage->setText(errorString);
    errorMessage->exec();
    server->clearError();
}

void ServerWindow::serverMessaged(const DSConvo::Protocol::MessageBroadcast &m)
{
    ui->recvPlainTextEdit->appendPlainText(DSConvo::formatMessage(m));
}
