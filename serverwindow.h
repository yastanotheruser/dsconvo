#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "dsconvoserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::MainWindow *ui;
    QMessageBox *errorMessage;
    DSConvoServer *server;

private slots:
    void toggleServer();
    void serverStatusChanged();

};
#endif // MAINWINDOW_H
