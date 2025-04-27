#include "../include/gamehistory.h"
#include "../include/database.h"
#include <QDebug>

// Initialize static instance
GameHistory* GameHistory::instance = nullptr;

GameHistory& GameHistory::getInstance()
{
    if (!instance) {
        instance = new GameHistory();
    }
    return *instance;
}

GameHistory::GameHistory(QObject *parent)
    : QObject(parent)
{
    // Load initial game records from database
    // (simplified for now, just an in-memory cache)
}

GameHistory::~GameHistory()
{
}

bool GameHistory::saveGame(const User &user, bool vsAI, const QString &opponent, GameLogic::GameState result, const std::vector<GameLogic::Move> &moves)
{
    // Save to database
    bool success = Database::getInstance().saveGame(user.getId(), vsAI, opponent, result, moves);
    
    if (success) {
        // Add to in-memory cache
        GameRecord record;
        record.gameId = gameRecords.size() + 1; // Simplified ID generation
        record.userId = user.getId();
        record.timestamp = QDateTime::currentDateTime();
        record.vsAI = vsAI;
        record.opponent = opponent;
        record.result = result;
        record.moves = moves;
        
        gameRecords.prepend(record); // Add to front for most recent first
        
        // Limit cache size
        if (gameRecords.size() > 100) {
            gameRecords.resize(100);
        }
    } else {
        emit gameHistoryError("Failed to save game to database");
    }
    
    return success;
}

QVector<GameHistory::GameRecord> GameHistory::getUserGameHistory(int userId) const
{
    QVector<GameRecord> userGames;
    
    // Check in-memory cache first
    for (const auto &record : gameRecords) {
        if (record.userId == userId) {
            userGames.append(record);
        }
    }
    
    // If not enough records in cache, get from database
    if (userGames.size() < 20) {
        QVector<QPair<int, QDateTime>> gameIds = Database::getInstance().getUserGameIds(userId);
        
        for (const auto &gameId : gameIds) {
            // Check if already in our list
            bool alreadyHave = false;
            for (const auto &existingGame : userGames) {
                if (existingGame.gameId == gameId.first) {
                    alreadyHave = true;
                    break;
                }
            }
            
            if (!alreadyHave) {
                // Get game details from database
                auto gameDetails = Database::getInstance().getGameDetails(gameId.first);
                
                // Get game record from database
                QSqlQuery query(Database::getInstance().getDatabase());
                query.prepare("SELECT vs_ai, opponent FROM games WHERE id = ?");
                query.addBindValue(gameId.first);
                
                if (query.exec() && query.next()) {
                    GameRecord record;
                    record.gameId = gameId.first;
                    record.userId = userId;
                    record.timestamp = gameId.second;
                    record.vsAI = query.value("vs_ai").toBool();
                    record.opponent = query.value("opponent").toString();
                    record.result = gameDetails.first;
                    record.moves = gameDetails.second;
                    
                    userGames.append(record);
                    
                    // Also add to cache if not too many
                    if (gameRecords.size() < 100) {
                        gameRecords.append(record);
                    }
                }
            }
            
            // Limit to 20 records
            if (userGames.size() >= 20) {
                break;
            }
        }
    }
    
    return userGames;
}

GameHistory::GameRecord GameHistory::getGameDetails(int gameId) const
{
    // Check in-memory cache first
    for (const auto &record : gameRecords) {
        if (record.gameId == gameId) {
            return record;
        }
    }
    
    // If not found in cache, get from database
    GameRecord record;
    record.gameId = gameId;
    
    // Get basic game info
    QSqlQuery query(Database::getInstance().getDatabase());
    query.prepare("SELECT user_id, timestamp, vs_ai, opponent, result FROM games WHERE id = ?");
    query.addBindValue(gameId);
    
    if (query.exec() && query.next()) {
        record.userId = query.value("user_id").toInt();
        record.timestamp = query.value("timestamp").toDateTime();
        record.vsAI = query.value("vs_ai").toBool();
        record.opponent = query.value("opponent").toString();
        record.result = static_cast<GameLogic::GameState>(query.value("result").toInt());
        
        // Get moves
        auto gameDetails = Database::getInstance().getGameDetails(gameId);
        record.moves = gameDetails.second;
        
        // Add to cache if not too large
        if (gameRecords.size() < 100) {
            gameRecords.append(record);
        }
    } else {
        emit gameHistoryError("Failed to retrieve game details");
    }
    
    return record;
}

QVector<User> GameHistory::getLeaderboard(int limit) const
{
    return Database::getInstance().getAllUsers().mid(0, limit);
} 