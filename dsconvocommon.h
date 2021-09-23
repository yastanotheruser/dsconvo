#ifndef DSCONVOCOMMON_H
#define DSCONVOCOMMON_H

#include <QPair>
#include <QHostAddress>
#include <QObject>
#include <QPlainTextEdit>
#include <QApplication>

namespace DSConvo {

typedef QPair<QHostAddress, quint16> AddressPort;
typedef QPair<QAbstractSocket::SocketError, QString> SocketErrorInfo;

struct CommandLineOptions {
    const QString *address;
    const QString *database;
    bool server;
};

constexpr quint16 DEFAULT_PORT = 5500;
constexpr quint16 MAX_MESSAGE = 255;
extern CommandLineOptions cmdline;
extern AddressPort serverAddress;

QString normalizeText(const QString&);
void initialize(QApplication &app);
bool validatePort(const QString&, quint16* = nullptr);
bool validateAddressPort(const QString&, AddressPort&, bool = false);
QString addressPortToString(const AddressPort&);

namespace QtUtil {

class PlainTextEditLimit : public QObject
{
    Q_OBJECT

public:
    explicit PlainTextEditLimit(int limit, QPlainTextEdit *parent);
    inline int limit() const { return limit_; }
    inline void setLimit(int l) { limit_ = l; }

signals:
    void limitExceeded(bool*);

private:
    int limit_;

private slots:
    void parentTextChanged();

};

} // namespace DSConvo::QtUtil

} // namespace DSConvo

#endif
