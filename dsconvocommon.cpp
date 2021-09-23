#include "dsconvocommon.h"
#include <QRegularExpression>
#include <QCommandLineParser>
#include "dsconvodatabase.h"

Q_DECLARE_METATYPE(::DSConvo::SocketErrorInfo);

namespace {

void parseCommandLine(const QApplication&);
void validateCommandLine();

} // anonymous

namespace DSConvo {

CommandLineOptions cmdline;
AddressPort serverAddress(QHostAddress::AnyIPv4, DEFAULT_PORT);

QString normalizeText(const QString &t)
{
    return t.trimmed();
}

void initialize(QApplication &app)
{
    QApplication::setApplicationName("dsconvo");
    QApplication::setApplicationDisplayName("dsconvo chat");
    QApplication::setApplicationVersion("0.1");
    parseCommandLine(app);
    DSConvo::Database::initializeDatabase(*cmdline.database);
}

bool validatePort(const QString &portString, quint16 *into)
{
    if (!portString.isEmpty()) {
        bool ok;
        quint32 port32 = portString.toUInt(&ok, 10);

        if (!ok || port32 == 0 || port32 > 65535) {
            return false;
        }

        if (into != nullptr) {
            *into = port32 & 0xffff;
        }
    } else if (into != nullptr) {
        *into = DSConvo::DEFAULT_PORT;
    }

    return true;
}

bool validateAddressPort(const QString &addrPortString,
                         AddressPort &addrPort,
                         bool targetServerAddress)
{
    QRegularExpression addrPortPattern("^\\s*(\\S*?)(?::(\\d{1,5}))?\\s*$");
    QRegularExpressionMatch match = addrPortPattern.match(addrPortString);

    if (!match.hasMatch()) {
        return false;
    }

    QString addressString = match.captured(1);
    QString portString = match.captured(2);
    QHostAddress address;
    quint16 port;

    if (targetServerAddress && addressString.isEmpty()) {
        return false;
    }

    if (!validatePort(portString, &port)) {
        return false;
    }

    if (!addressString.isEmpty()) {
        address = QHostAddress(addressString);
        if (address.isNull()) {
            return false;
        }
    } else {
        address = QHostAddress::AnyIPv4;
    }

    addrPort.first = address;
    addrPort.second = port;
    return true;
}

QString addressPortToString(const AddressPort &addrPort)
{
    QString res;
    QTextStream(&res) << addrPort.first.toString() << ":"
                      << QString::number(addrPort.second);
    return res;
}

namespace QtUtil {

PlainTextEditLimit::PlainTextEditLimit(int limit, QPlainTextEdit *parent)
    : QObject(parent)
    , limit_(limit)
{
    connect(parent, SIGNAL(textChanged()), this, SLOT(parentTextChanged()));
}

void PlainTextEditLimit::parentTextChanged()
{
    auto *p = qobject_cast<QPlainTextEdit*>(parent());
    Q_ASSERT(p != nullptr);
    QString text = p->toPlainText();

    if (limit_ <= 0 || text.length() <= limit_) {
        return;
    }

    bool allow = false;
    emit limitExceeded(&allow);

    if (allow) {
        return;
    }

    p->setPlainText(text.left(limit_));
    QTextCursor cursor = p->textCursor();
    cursor.movePosition(QTextCursor::End);
    p->setTextCursor(cursor);
}

} // namespace DSConvo::QtUtil

} // namespace DSConvo

namespace {

void parseCommandLine(const QApplication &app)
{
    using DSConvo::cmdline;

    QCommandLineParser parser;
    parser.setApplicationDescription("DSConvo chat");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("address", app.tr("Dirección de escucha o conexión"));

    QCommandLineOption databaseOption(QStringList() << "d" << "database",
                                      app.tr("Base de datos SQLite3"),
                                      app.tr("file"));
    databaseOption.setDefaultValue(DSConvo::Database::DEFAULT_DATABASE);
    parser.addOption(databaseOption);

    QCommandLineOption serverOption(QStringList() << "l" << "server",
                                    app.tr("Modo servidor"));
    parser.addOption(serverOption);

    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        cmdline.address = new QString(args.at(0));
    }

    cmdline.database = new QString(parser.value(databaseOption));
    cmdline.server = parser.isSet(serverOption);
    validateCommandLine();
}

void validateCommandLine()
{
    using DSConvo::cmdline;
    using DSConvo::serverAddress;

    if (DSConvo::cmdline.address != nullptr) {
        if (!DSConvo::validateAddressPort(*cmdline.address, serverAddress,
                                          !cmdline.server))
        {
            qCritical("Dirección inválida");
            exit(EXIT_FAILURE);
        }
    } else {
        if (!cmdline.server) {
            serverAddress.first = QHostAddress::LocalHost;
        }
    }
}

} // anonymous
