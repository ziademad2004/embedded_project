#ifndef USER_H
#define USER_H

#include <QString>
#include <QVector>
#include <QDateTime>

struct GameRecord {
    QString date;
    QString result;
};

class User {
public:
    User();
    User(const QString &username, const QString &password);

    QString getUsername() const;
    bool checkPassword(const QString &password) const;
    
    // Statistics
    int getTotalGames() const;
    int getWins() const;
    int getLosses() const;
    int getDraws() const;
    int getVsAI() const;
    int getVsPlayers() const;
    int getWinRate() const;
    int getBestStreak() const;
    QVector<GameRecord> getGameHistory() const;
    
    // Update statistics
    void addGame(const QString &result, const QString &opponent, const QString &difficulty = "");
    
    // Add a game with a custom date (for loading from database)
    void addGameWithDate(const QString &result, const QString &opponent, const QString &date, const QString &difficulty = "");

private:
    QString m_username;
    QString m_password;
    
    // Statistics
    int m_totalGames;
    int m_wins;
    int m_losses;
    int m_draws;
    int m_vsAI;
    int m_vsPlayers;
    int m_winRate;
    int m_bestStreak;
    int m_currentStreak;
    QVector<GameRecord> m_gameHistory;
};

#endif // USER_H
