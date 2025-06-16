#include "../include/database.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>
#include <algorithm>

Database::Database(QObject *parent)
    : QObject(parent)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_dbPath = appDataPath + "/tictactoe.json";
}

Database::~Database() {
}

bool Database::saveUsers(const QVector<User*> &users) {
    QJsonArray usersArray;
    
    const auto& usersRef = users;
    for (const User* user : usersRef) {
        QJsonObject userObj;
        userObj["username"] = user->getUsername();
        // We can't access the password directly, so we'll skip saving it
        // userObj["password"] = user->getPassword(); // In a real app, this should be hashed
        
        QJsonObject stats;
        stats["totalGames"] = user->getTotalGames();
        stats["wins"] = user->getWins();
        stats["losses"] = user->getLosses();
        stats["draws"] = user->getDraws();
        stats["vsAI"] = user->getVsAI();
        stats["vsPlayers"] = user->getVsPlayers();
        stats["winRate"] = user->getWinRate();
        stats["bestStreak"] = user->getBestStreak();
        
        QJsonArray historyArray;
        const auto& gameHistory = user->getGameHistory();
        for (const GameRecord &record : gameHistory) {
            QJsonObject historyObj;
            historyObj["date"] = record.date;
            historyObj["result"] = record.result;
            historyArray.append(historyObj);
        }
        
        stats["gameHistory"] = historyArray;
        userObj["stats"] = stats;
        
        usersArray.append(userObj);
    }
    
    QJsonDocument doc(usersArray);
    QFile file(m_dbPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

QVector<User*> Database::loadUsers() {
    QVector<User*> users;
    QFile file(m_dbPath);
    
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        // Create default users if no database exists
        users.append(new User("player1", "pass123"));
        users.append(new User("player2", "pass123"));
        
        // No default demo user - leaderboard will be populated with real player data
        
        return users;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray usersArray = doc.array();
    
    const auto& usersArrayRef = usersArray;
    for (const QJsonValue &value : usersArrayRef) {
        QJsonObject userObj = value.toObject();
        
        QString username = userObj["username"].toString();
        // We can't access the password directly, so we'll use an empty password
        // The User constructor will need to handle this appropriately
        User* user = new User(username, "defaultpassword");  // Use a default password
        
        if (userObj.contains("stats")) {
            QJsonObject stats = userObj["stats"].toObject();
            
            // Load game history
            if (stats.contains("gameHistory")) {
                QJsonArray historyArray = stats["gameHistory"].toArray();
                
                const auto& historyArrayRef = historyArray;
                for (const QJsonValue &historyValue : historyArrayRef) {
                    QJsonObject historyObj = historyValue.toObject();
                    
                    QString date = historyObj["date"].toString();
                    QString result = historyObj["result"].toString();
                    
                    // Parse result to determine game type and outcome
                    if (result.contains("vs AI")) {
                        QString difficulty = "medium";
                        if (result.contains("easy")) {
                            difficulty = "easy";
                        } else if (result.contains("hard")) {
                            difficulty = "hard";
                        } else if (result.contains("expert")) {
                            difficulty = "expert";
                        }
                        
                        if (result.startsWith("Win")) {
                            user->addGameWithDate("win", "ai", date, difficulty);
                        } else if (result.startsWith("Loss")) {
                            user->addGameWithDate("loss", "ai", date, difficulty);
                        } else {
                            user->addGameWithDate("draw", "ai", date, difficulty);
                        }
                    } else {
                        QString opponent = result.section("vs ", 1);
                        
                        if (result.startsWith("Win")) {
                            user->addGameWithDate("win", opponent, date);
                        } else if (result.startsWith("Loss")) {
                            user->addGameWithDate("loss", opponent, date);
                        } else {
                            user->addGameWithDate("draw", opponent, date);
                        }
                    }
                }
            }
        }
        
        users.append(user);
    }
    
    return users;
}

bool Database::saveGame(const QString &playerX, const QString &playerO, const QString &result) {
    // In a real application, this would save the game to a database
    // For this demo, we'll just return true
    return true;
}

QVector<LeaderboardEntry> Database::getLeaderboard(const QVector<User*> &users) const {
    QVector<LeaderboardEntry> leaderboard;
    
    // Populate leaderboard entries from users
    // Use index-based loop to avoid Qt container detachment warning
    for (int i = 0; i < users.size(); ++i) {
        const User* user = users.at(i);
        // Only include users who have played at least one game
        if (user->getTotalGames() > 0) {
            LeaderboardEntry entry;
            entry.username = user->getUsername();
            entry.wins = user->getWins();
            entry.totalGames = user->getTotalGames();
            entry.winRate = user->getWinRate();
            entry.bestStreak = user->getBestStreak();
            
            // Calculate score for ranking
            entry.score = calculatePlayerScore(entry.wins, entry.totalGames, entry.winRate, entry.bestStreak);
            
            leaderboard.append(entry);
        }
    }
    
    // Sort leaderboard by score (descending)
    std::sort(leaderboard.begin(), leaderboard.end(), 
              [](const LeaderboardEntry &a, const LeaderboardEntry &b) {
                  return a.score > b.score;
              });
    
    return leaderboard;
}

int Database::calculatePlayerScore(int wins, int totalGames, int winRate, int bestStreak) const {
    // Calculate a composite score based on multiple factors
    // This formula weights different aspects of player performance
    
    // Base score from win rate (0-100)
    int score = winRate;
    
    // Add bonus for total games played (experience factor)
    // More games played = more reliable stats and more experience
    score += std::min(totalGames / 2, 50); // Cap at 50 points (100 games)
    
    // Add bonus for absolute number of wins
    score += std::min(wins * 3, 75); // Cap at 75 points (25 wins)
    
    // Add bonus for best streak
    score += std::min(bestStreak * 5, 50); // Cap at 50 points (10 game streak)
    
    // Total possible score: 275 points
    return score;
}
