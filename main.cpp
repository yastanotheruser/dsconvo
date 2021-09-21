#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "dsconvocommon.h"
#include "dsconvoserver.h"
#include "serverwindow.h"
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DSConvo::initialize(a);
    QMainWindow *w = DSConvo::cmdline.server ?
                static_cast<QMainWindow*>(new ServerWindow()) :
                static_cast<QMainWindow*>(new ClientWindow());
    w->show();
    return a.exec();
}
