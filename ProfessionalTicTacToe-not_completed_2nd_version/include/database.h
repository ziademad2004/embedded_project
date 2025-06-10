#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QVector>
#include <QString>
#include "user.h"

class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool saveUsers(const QVector<User*> &users);
    QVector<User*> loadUsers();
    
    bool saveGame(const QString &playerX, const QString &playerO, const QString &result);
    QVector<QPair<QString, QString>> getLeaderboard() const;

private:
    QString m_dbPath;
};

#endif // DATABASE_H
