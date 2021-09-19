#include "serverwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QAbstractSocket>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , errorMessage(new QMessageBox(this))
    , server(new DSConvoServer)
{
    ui->setupUi(this);
    ui->portLineEdit->setText(QString::number(DSConvoServer::DEFAULT_PORT));
    errorMessage->setIcon(QMessageBox::Critical);
    errorMessage->setWindowTitle("Error");
    errorMessage->setStandardButtons(QMessageBox::Close);

    connect(ui->listenPushButton, SIGNAL(clicked()), this, SLOT(toggleServer()));
    connect(server, SIGNAL(statusChanged()), this, SLOT(serverStatusChanged()));
}

ServerWindow::~ServerWindow()
{
    delete ui;
    delete server;
}

void ServerWindow::toggleServer()
{
    if (!server->listening()) {
        ui->listenPushButton->setDisabled(true);
        if (server->listen()) {
            ui->portLineEdit->setDisabled(true);
            ui->sendTextEdit->setDisabled(false);
            ui->sendPushButton->setDisabled(false);
            ui->listenPushButton->setText(tr("Detener"));
        }

        ui->listenPushButton->setDisabled(false);
    } else {
        ui->listenPushButton->setDisabled(true);
        server->close();
        ui->portLineEdit->setDisabled(false);
        ui->listenPushButton->setDisabled(false);
        ui->sendTextEdit->setDisabled(true);
        ui->sendPushButton->setDisabled(true);
        ui->listenPushButton->setText(tr("Iniciar servidor"));
    }
}

void ServerWindow::serverStatusChanged()
{
    if (server->status() != DSConvoServer::Status::Error) {
        statusBar()->showMessage(server->statusString());
        return;
    }

    auto errorInfo = qvariant_cast<SocketErrorInfo>(server->statusData());
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
