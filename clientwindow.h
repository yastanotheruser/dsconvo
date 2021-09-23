#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>
#include <QTimer>
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

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

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
    void initClientConnection();
    void updateCompleter();

    Ui::ClientWindow *ui;
    QMessageBox *errorMessage;
    UiMode uiMode;
    DSConvoClient *client;
    QTimer *completerTimer;
    bool canUpdateCompleter;

private slots:
    void toggleConnection();
    void sendMessage();
    void clientStateChanged();
    void clientConnGreeted();
    void clientConnMessaged(const DSConvo::Protocol::MessageBroadcast &m);
    void clientConnErrorOccurred(DSConvoClientConnection::Error e);
    void completerTimerTimeout();

};
#endif
