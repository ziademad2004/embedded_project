#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include "user.h"

// Define a struct to hold leaderboard entry data
struct LeaderboardEntry {
    QString username;
    int wins;
    int totalGames;
    int winRate;
    int bestStreak;
    int score; // Calculated score for ranking
};

class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool saveUsers(const QVector<User*> &users);
    QVector<User*> loadUsers();
    
    bool saveGame(const QString &playerX, const QString &playerO, const QString &result);
    
    // Updated to return comprehensive leaderboard data
    QVector<LeaderboardEntry> getLeaderboard(const QVector<User*> &users) const;
    
    // Override setProperty for testing
    bool setProperty(const char *name, const QVariant &value);

private:
    QString m_dbPath;
    
    // Helper method to calculate player score for ranking
    int calculatePlayerScore(int wins, int totalGames, int winRate, int bestStreak) const;
};

#endif // DATABASE_H
