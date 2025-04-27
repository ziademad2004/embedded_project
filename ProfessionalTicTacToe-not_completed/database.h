#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>
#include <QVector>
#include "user.h"
#include "gamelogic.h"

class Database : public QObject
{
    Q_OBJECT

public:
    static Database& getInstance();
    
    // User management
    bool addUser(const QString &username, const QString &email, const QString &passwordHash);
    User getUserById(int id) const;
    User getUserByUsername(const QString &username) const;
    QVector<User> getAllUsers() const;
    bool updateUserStats(int userId, int wins, int losses, int draws);
    bool updateLastLogin(int userId, const QDateTime &lastLogin);
    QString getPasswordHash(const QString &username) const;
    
    // Game history
    bool saveGame(int userId, bool vsAI, const QString &opponent, GameLogic::GameState result, const std::vector<GameLogic::Move> &moves);
    QVector<QPair<int, QDateTime>> getUserGameIds(int userId) const;
    QPair<GameLogic::GameState, std::vector<GameLogic::Move>> getGameDetails(int gameId) const;
    
    // Database access (for GameHistory class)
    QSqlDatabase getDatabase() const { return db; }
    
signals:
    void databaseError(const QString &errorMessage);

private:
    Database(QObject *parent = nullptr);
    ~Database();
    
    // Disable copy constructor and assignment operator
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // We'll use a singleton pattern
    static Database* instance;
    
    QSqlDatabase db;
    
    bool initializeDatabase();
    bool executeSqlQuery(const QString &query);
};

#endif // DATABASE_H 