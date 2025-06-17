// clazy:skip

#include <QTest>
#include <QObject>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <functional>
#include "../include/gamelogic.h"
#include "../include/aiopponent.h"
#include "../include/database.h"
#include "../include/authentication.h"
#include "../include/user.h"

class TestPerformance : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    
    // Response Time Tests
    void testAIResponseTime();
    void testDatabaseResponseTime();
    void testAuthenticationResponseTime();
    
    // Resource Utilization Tests
    void testMemoryUsage();
    void testCPUUsage();
    
    // Stress Tests
    void testGameLogicStress();
    void testDatabaseStress();

private:
    GameLogic *gameLogic;
    AIOpponent *aiOpponent;
    Database *database;
    Authentication *auth;
    
    // Helper methods
    void waitForAIMove(int timeout = 1000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(timeout);
        loop.exec();
    }
    
    qint64 getCurrentMemoryUsage() {
        QProcess process;
        qint64 memoryUsage = 0;
        
#ifdef Q_OS_WIN
        // Windows memory usage check
        process.start("wmic", QStringList() << "process" << "where" << 
                     QString("ProcessId=%1").arg(QCoreApplication::applicationPid()) << 
                     "get" << "WorkingSetSize");
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        QStringList lines = output.split("\n", Qt::SkipEmptyParts);
        if (lines.size() >= 2) {
            memoryUsage = lines[1].trimmed().toLongLong();
        }
#else
        // Linux/Unix memory usage check
        process.start("ps", QStringList() << "-o" << "rss" << "-p" << 
                     QString::number(QCoreApplication::applicationPid()));
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        QStringList lines = output.split("\n", Qt::SkipEmptyParts);
        if (lines.size() >= 2) {
            memoryUsage = lines[1].trimmed().toLongLong() * 1024; // Convert KB to bytes
        }
#endif
        return memoryUsage;
    }
    
    double measureOperationTime(std::function<void()> operation) {
        QElapsedTimer timer;
        timer.start();
        operation();
        return timer.elapsed();
    }
};

void TestPerformance::init()
{
    gameLogic = new GameLogic();
    aiOpponent = new AIOpponent();
    database = new Database();
    auth = new Authentication();
    
    aiOpponent->setGameLogic(gameLogic);
}

void TestPerformance::cleanup()
{
    delete gameLogic;
    delete aiOpponent;
    delete database;
    delete auth;
}

void TestPerformance::testAIResponseTime()
{
    qDebug() << "=== AI Response Time Test ===";
    
    // Test different difficulty levels
    QVector<GameLogic::AIDifficulty> difficulties = {
        GameLogic::AIDifficulty::Easy,
        GameLogic::AIDifficulty::Medium,
        GameLogic::AIDifficulty::Hard,
        GameLogic::AIDifficulty::Expert
    };
    
    QVector<QString> difficultyNames = {
        "Easy", "Medium", "Hard", "Expert"
    };
    
    for (int i = 0; i < difficulties.size(); i++) {
        gameLogic->resetBoard();
        gameLogic->setAIDifficulty(difficulties[i]);
        
        QElapsedTimer timer;
        timer.start();
        
        aiOpponent->makeMove();
        waitForAIMove();
        
        qint64 elapsed = timer.elapsed();
        qDebug() << "AI Difficulty:" << difficultyNames[i] << "- Response Time:" << elapsed << "ms";
        
        // Verify reasonable response times
        // Even expert should respond within 3 seconds
        QVERIFY2(elapsed < 3000, "AI response time exceeded 3 seconds");
    }
    
    // Test AI response time at different game stages
    qDebug() << "Testing AI response time at different game stages:";
    gameLogic->resetBoard();
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Expert);
    
    // Early game (empty board)
    QElapsedTimer timer;
    timer.start();
    aiOpponent->makeMove();
    waitForAIMove();
    qDebug() << "Early game (empty board):" << timer.elapsed() << "ms";
    
    // Mid game (4 moves)
    gameLogic->resetBoard();
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(1); // X
    
    timer.start();
    aiOpponent->makeMove();
    waitForAIMove();
    qDebug() << "Mid game (4 moves):" << timer.elapsed() << "ms";
    
    // Late game (6 moves)
    gameLogic->resetBoard();
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(5); // O
    gameLogic->makeMove(6); // X
    
    timer.start();
    aiOpponent->makeMove();
    waitForAIMove();
    qDebug() << "Late game (6 moves):" << timer.elapsed() << "ms";
}

void TestPerformance::testDatabaseResponseTime()
{
    qDebug() << "=== Database Response Time Test ===";
    
    // Create test users
    QVector<User*> users;
    const int userCount = 100;
    
    QElapsedTimer timer;
    
    // Measure user creation time
    timer.start();
    for (int i = 0; i < userCount; i++) {
        QString username = QString("perfuser%1").arg(i);
        User* user = new User(username, "password");
        
        // Add some game data
        int games = 5;
        for (int j = 0; j < games; j++) {
            user->addGame(j % 2 == 0 ? "win" : "loss", "ai");
        }
        
        users.append(user);
    }
    qDebug() << "Time to create" << userCount << "users with game data:" << timer.elapsed() << "ms";
    
    // Measure save time
    timer.start();
    bool saveResult = database->saveUsers(users);
    qint64 saveTime = timer.elapsed();
    qDebug() << "Time to save" << userCount << "users:" << saveTime << "ms";
    QVERIFY(saveResult);
    QVERIFY2(saveTime < 5000, "Database save time exceeded 5 seconds");
    
    // Measure load time
    timer.start();
    QVector<User*> loadedUsers = database->loadUsers();
    qint64 loadTime = timer.elapsed();
    qDebug() << "Time to load users:" << loadTime << "ms";
    QVERIFY(!loadedUsers.isEmpty());
    QVERIFY2(loadTime < 5000, "Database load time exceeded 5 seconds");
    
    // Measure leaderboard generation time
    timer.start();
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);
    qint64 leaderboardTime = timer.elapsed();
    qDebug() << "Time to generate leaderboard:" << leaderboardTime << "ms";
    QVERIFY(!leaderboard.isEmpty());
    QVERIFY2(leaderboardTime < 2000, "Leaderboard generation time exceeded 2 seconds");
    
    // Clean up
    for (User* user : users) {
        delete user;
    }
    
    for (User* user : loadedUsers) {
        delete user;
    }
}

void TestPerformance::testAuthenticationResponseTime()
{
    qDebug() << "=== Authentication Response Time Test ===";
    
    QElapsedTimer timer;
    
    // Measure registration time
    QString username = "perfauthuser";
    QString password = "Password123!";
    
    timer.start();
    bool registerResult = auth->registerUser(username, password);
    qint64 registerTime = timer.elapsed();
    qDebug() << "User registration time:" << registerTime << "ms";
    QVERIFY(registerResult || auth->getErrorMessage().contains("already exists"));
    QVERIFY2(registerTime < 1000, "Registration time exceeded 1 second");
    
    // Measure login time
    timer.start();
    bool loginResult = auth->login(username, password);
    qint64 loginTime = timer.elapsed();
    qDebug() << "User login time:" << loginTime << "ms";
    QVERIFY(loginResult);
    QVERIFY2(loginTime < 1000, "Login time exceeded 1 second");
    
    // Measure password hashing time (this is the most intensive part)
    timer.start();
    QString hashedPassword = Authentication::hashPassword(password);
    qint64 hashTime = timer.elapsed();
    qDebug() << "Password hashing time:" << hashTime << "ms";
    QVERIFY(!hashedPassword.isEmpty());
    QVERIFY2(hashTime < 1000, "Password hashing time exceeded 1 second");
}

void TestPerformance::testMemoryUsage()
{
    qDebug() << "=== Memory Usage Test ===";
    
    // Baseline memory usage
    qint64 baselineMemory = getCurrentMemoryUsage();
    qDebug() << "Baseline memory usage:" << baselineMemory / 1024 << "KB";
    
    // Memory usage after creating game objects
    GameLogic* testGameLogic = new GameLogic();
    AIOpponent* testAI = new AIOpponent();
    testAI->setGameLogic(testGameLogic);
    
    qint64 gameObjectsMemory = getCurrentMemoryUsage();
    qDebug() << "Memory usage after creating game objects:" << gameObjectsMemory / 1024 << "KB";
    qDebug() << "Delta:" << (gameObjectsMemory - baselineMemory) / 1024 << "KB";
    
    // Memory usage after creating many users
    QVector<User*> users;
    const int userCount = 1000;
    
    for (int i = 0; i < userCount; i++) {
        QString username = QString("memuser%1").arg(i);
        User* user = new User(username, "password");
        user->addGame("win", "ai");
        users.append(user);
    }
    
    qint64 manyUsersMemory = getCurrentMemoryUsage();
    qDebug() << "Memory usage after creating" << userCount << "users:" << manyUsersMemory / 1024 << "KB";
    qDebug() << "Delta from baseline:" << (manyUsersMemory - baselineMemory) / 1024 << "KB";
    qDebug() << "Average per user:" << (manyUsersMemory - gameObjectsMemory) / userCount << "bytes";
    
    // Clean up
    delete testGameLogic;
    delete testAI;
    
    for (User* user : users) {
        delete user;
    }
    
    // Verify memory is released
    qint64 finalMemory = getCurrentMemoryUsage();
    qDebug() << "Final memory usage after cleanup:" << finalMemory / 1024 << "KB";
    qDebug() << "Delta from baseline:" << (finalMemory - baselineMemory) / 1024 << "KB";
    
    // Memory should be within reasonable bounds of the baseline
    // Allow for some overhead due to memory allocation patterns
    QVERIFY2((finalMemory - baselineMemory) < 5 * 1024 * 1024, "Memory leak detected");
}

void TestPerformance::testCPUUsage()
{
    qDebug() << "=== CPU Usage Test ===";
    
    // Test CPU usage for AI operations at Expert level
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Expert);
    
    QElapsedTimer timer;
    timer.start();
    
    // Run multiple AI moves to measure CPU usage
    const int iterations = 10;
    for (int i = 0; i < iterations; i++) {
        gameLogic->resetBoard();
        
        // Make some moves to create a more complex board
        gameLogic->makeMove(0); // X
        gameLogic->makeMove(4); // O
        gameLogic->makeMove(1); // X
        
        // Now let AI think
        aiOpponent->makeMove();
        waitForAIMove();
    }
    
    qint64 totalTime = timer.elapsed();
    qDebug() << "Total time for" << iterations << "AI moves:" << totalTime << "ms";
    qDebug() << "Average time per AI move:" << totalTime / iterations << "ms";
    
    // AI operations should complete within reasonable time
    // Increased threshold to 2000ms to account for slower machines
    QVERIFY2(totalTime / iterations < 2000, "Average AI move time exceeded 2 seconds");
}

void TestPerformance::testGameLogicStress()
{
    qDebug() << "=== Game Logic Stress Test ===";
    
    QElapsedTimer timer;
    timer.start();
    
    // Play many games in sequence
    const int gameCount = 100;
    for (int game = 0; game < gameCount; game++) {
        gameLogic->resetBoard();
        
        // Play a full game with random moves
        bool gameOver = false;
        while (!gameOver) {
            // Find an empty cell
            QVector<int> emptyCells;
            for (int i = 0; i < 9; i++) {
                if (gameLogic->getCellState(i) == GameLogic::Player::None) {
                    emptyCells.append(i);
                }
            }
            
            if (emptyCells.isEmpty()) {
                gameOver = true;
            } else {
                // Make a random move
                int randomIndex = QRandomGenerator::global()->bounded(emptyCells.size());
                int cellToPlay = emptyCells[randomIndex];
                gameLogic->makeMove(cellToPlay);
                
                // Check if game is over
                if (gameLogic->getGameResult() != GameLogic::GameResult::InProgress) {
                    gameOver = true;
                }
            }
        }
    }
    
    qint64 totalTime = timer.elapsed();
    qDebug() << "Time to play" << gameCount << "full games:" << totalTime << "ms";
    qDebug() << "Average time per game:" << totalTime / gameCount << "ms";
    
    // Games should play quickly
    QVERIFY2(totalTime / gameCount < 50, "Average game time exceeded 50ms");
}

void TestPerformance::testDatabaseStress()
{
    qDebug() << "=== Database Stress Test ===";
    
    // Create a large number of users with extensive game histories
    QVector<User*> users;
    const int userCount = 500;
    const int gamesPerUser = 20;
    
    QElapsedTimer timer;
    timer.start();
    
    // Create users with game histories
    for (int i = 0; i < userCount; i++) {
        QString username = QString("stressuser%1").arg(i);
        User* user = new User(username, "password");
        
        // Add game history
        for (int j = 0; j < gamesPerUser; j++) {
            bool isWin = QRandomGenerator::global()->bounded(2) == 1;
            QString opponent = QRandomGenerator::global()->bounded(2) == 1 ? 
                              "ai" : QString("stressuser%1").arg(QRandomGenerator::global()->bounded(userCount));
            QString difficulty = "medium";
            
            user->addGame(isWin ? "win" : "loss", opponent, difficulty);
        }
        
        users.append(user);
    }
    
    qint64 creationTime = timer.elapsed();
    qDebug() << "Time to create" << userCount << "users with" << gamesPerUser << "games each:" 
             << creationTime << "ms";
    
    // Test database save performance
    timer.restart();
    bool saveResult = database->saveUsers(users);
    qint64 saveTime = timer.elapsed();
    qDebug() << "Time to save" << userCount << "users with extensive game histories:" << saveTime << "ms";
    QVERIFY(saveResult);
    
    // Test database load performance
    timer.restart();
    QVector<User*> loadedUsers = database->loadUsers();
    qint64 loadTime = timer.elapsed();
    qDebug() << "Time to load all users with game histories:" << loadTime << "ms";
    QVERIFY(!loadedUsers.isEmpty());
    
    // Test leaderboard generation with large dataset
    timer.restart();
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);
    qint64 leaderboardTime = timer.elapsed();
    qDebug() << "Time to generate leaderboard with" << userCount << "users:" << leaderboardTime << "ms";
    QVERIFY(!leaderboard.isEmpty());
    
    // Clean up
    for (User* user : users) {
        delete user;
    }
    
    for (User* user : loadedUsers) {
        delete user;
    }
}

QTEST_MAIN(TestPerformance)
#include "test_performance.moc"