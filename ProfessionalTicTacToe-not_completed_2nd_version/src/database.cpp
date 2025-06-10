#include "../include/database.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>

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
        
        User* demoUser = new User("demo", "demo");
        demoUser->addGame("win", "ai", "medium");
        demoUser->addGame("win", "ai", "easy");
        demoUser->addGame("loss", "ai", "hard");
        demoUser->addGame("draw", "ai", "medium");
        demoUser->addGame("win", "player2");
        demoUser->addGame("loss", "player1");
        demoUser->addGame("win", "ai", "medium");
        demoUser->addGame("win", "ai", "easy");
        demoUser->addGame("win", "ai", "medium");
        demoUser->addGame("loss", "ai", "expert");
        demoUser->addGame("draw", "ai", "hard");
        demoUser->addGame("win", "player1");
        demoUser->addGame("win", "player2");
        demoUser->addGame("draw", "player1");
        demoUser->addGame("loss", "player2");
        users.append(demoUser);
        
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

QVector<QPair<QString, QString>> Database::getLeaderboard() const {
    QVector<QPair<QString, QString>> leaderboard;
    
    // In a real application, this would query the database for leaderboard data
    // For this demo, we'll return the same data as the HTML version
    leaderboard.append(qMakePair(QString("Player123"), QString("Wins: 28 | Win Rate: 85%")));
    leaderboard.append(qMakePair(QString("GameMaster"), QString("Wins: 24 | Win Rate: 80%")));
    leaderboard.append(qMakePair(QString("TicTacPro"), QString("Wins: 19 | Win Rate: 76%")));
    
    return leaderboard;
}
