#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "dsconvoserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::ServerWindow *ui;
    QMessageBox *errorMessage;
    DSConvoServer *server;

private slots:
    void toggleServer();
    void broadcast();
    void serverStateChanged();
    void serverMessaged(const DSConvo::Protocol::MessageBroadcast &m);

};
#endif
