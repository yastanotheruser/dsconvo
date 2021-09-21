#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>
#include "dsconvoclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private:
    enum UiMode {
        CanConnect,
        Connecting,
        CanSend,
        Disconnecting,
    };

    void switchUi(UiMode mode);
    void doConnect();
    void doDisconnect();

    Ui::ClientWindow *ui;
    QMessageBox *errorMessage;
    UiMode uiMode;
    DSConvoClient *client;

private slots:
    void toggleConnection();
    void clientStatusChanged();

};
#endif
