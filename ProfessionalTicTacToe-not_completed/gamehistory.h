#ifndef GAMEHISTORY_H
#define GAMEHISTORY_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QSqlQuery>
#include "user.h"
#include "gamelogic.h"

class GameHistory : public QObject
{
    Q_OBJECT

public:
    static GameHistory& getInstance();
    
    struct GameRecord {
        int gameId;
        int userId;
        QDateTime timestamp;
        bool vsAI;
        QString opponent;
        GameLogic::GameState result;
        std::vector<GameLogic::Move> moves;
    };
    
    bool saveGame(const User &user, bool vsAI, const QString &opponent, GameLogic::GameState result, const std::vector<GameLogic::Move> &moves);
    QVector<GameRecord> getUserGameHistory(int userId) const;
    GameRecord getGameDetails(int gameId) const;
    QVector<User> getLeaderboard(int limit = 10) const;
    
signals:
    void gameHistoryError(const QString &errorMessage);

private:
    GameHistory(QObject *parent = nullptr);
    ~GameHistory();
    
    // Disable copy constructor and assignment operator
    GameHistory(const GameHistory&) = delete;
    GameHistory& operator=(const GameHistory&) = delete;
    
    // We'll use a singleton pattern
    static GameHistory* instance;
    
    // In-memory cache of recent games
    QVector<GameRecord> gameRecords;
};

#endif // GAMEHISTORY_H 