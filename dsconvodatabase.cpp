#include "dsconvodatabase.h"
#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>
#include <iterator>

namespace DSConvo {

namespace Database {

QString databaseName;
bool databaseEnabled = false;

bool initializeDatabase(const QString &name = DEFAULT_DATABASE)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DATABASE);
    databaseName = name;
    db.setDatabaseName(databaseName);
    db.setConnectOptions("QSQLITE_OPEN_READONLY");

    if (!db.open()) {
        std::string errorString = db.lastError().text().toStdString();
        qDebug("[DEBUG] [DSConvo::Database::initializeDatabase] failed to open (%s)",
               errorString.c_str());
        databaseEnabled = false;
    } else {
        qDebug("[DEBUG] [DSConvo::Database::initializeDatabase] db open");
        databaseEnabled = true;
    }

    return databaseEnabled;
}

bool database(QSqlDatabase &db)
{
    if (!databaseEnabled) {
        return false;
    }

    db = QSqlDatabase::database(DATABASE);
    if (!db.isOpen()) {
        return false;
    }

    return true;
}

bool queryUsers(UsersFunction f)
{
    QSqlDatabase db;
    if (!database(db)) {
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT username, displayName FROM Users")) {
        return false;
    }

    while (query.next()) {
        f({query.value(0).toString(), query.value(1).toString()});
    }

    return true;
}

bool queryUsers(QList<User> &list)
{
    return queryUsers([&list](const User &u) { list.append(u); });
}

} // namepsace DSConvo::Database

} // namespace DSConvo
