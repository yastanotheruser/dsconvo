#ifndef DSCONVODATABASE_H
#define DSCONVODATABASE_H

#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <functional>

namespace DSConvo {

namespace Database {

struct User {
    QString username;
    QString displayName;
};

typedef std::function<void(const User&)> UsersFunction;

constexpr const char DATABASE[] = "dssconvo";
constexpr const char DEFAULT_DATABASE[] = "dsconvo.sqlite";
extern QString databaseName;
extern bool databaseEnabled;

bool initializeDatabase(const QString&);
bool database(QSqlDatabase&);
bool queryUsers(UsersFunction);
bool queryUsers(QList<User>&);

} // namespace DSConvo::Database

} // namespace DSConvo

#endif
