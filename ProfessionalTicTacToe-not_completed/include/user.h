#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

class User
{
public:
    User();
    User(int id, const QString &username, const QString &email);
    
    // Getters
    int getId() const;
    QString getUsername() const;
    QString getEmail() const;
    int getGamesPlayed() const;
    int getWins() const;
    int getLosses() const;
    int getDraws() const;
    QDateTime getLastLogin() const;
    
    // Setters
    void setId(int id);
    void setUsername(const QString &username);
    void setEmail(const QString &email);
    void setGamesPlayed(int games);
    void setWins(int wins);
    void setLosses(int losses);
    void setDraws(int draws);
    void setLastLogin(const QDateTime &lastLogin);
    
    // Statistics methods
    void incrementGamesPlayed();
    void incrementWins();
    void incrementLosses();
    void incrementDraws();
    
    // Validation
    bool isValid() const;
    
private:
    int id;
    QString username;
    QString email;
    int gamesPlayed;
    int wins;
    int losses;
    int draws;
    QDateTime lastLogin;
};

#endif // USER_H 