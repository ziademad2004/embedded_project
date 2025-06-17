// clazy:skip

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include "../include/gamelogic.h"
#include "../include/aiopponent.h"
#include "../include/authentication.h"
#include "../include/database.h"
#include "../include/user.h"

class TestIntegration : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    
    void testAuthenticationToGameFlow();
    void testGameCompletionToHistory();
    void testAIIntegration();
    void testDatabaseIntegration();

private:
    GameLogic *gameLogic;
    AIOpponent *aiOpponent;
    Authentication *auth;
    Database *database;
    
    // Helper method to wait for AI move to complete
    void waitForAIMove(int timeout = 1000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(timeout);
        loop.exec();
    }
};

void TestIntegration::init()
{
    gameLogic = new GameLogic();
    aiOpponent = new AIOpponent();
    auth = new Authentication();
    database = new Database();
    
    aiOpponent->setGameLogic(gameLogic);
}

void TestIntegration::cleanup()
{
    delete gameLogic;
    delete aiOpponent;
    delete auth;
    delete database;
}

void TestIntegration::testAuthenticationToGameFlow()
{
    // 1. Register and login
    QString username = "testplayer";
    QString password = "password123";
    
    // Make sure user doesn't already exist (might be from previous test run)
    bool userRegistered = true;
    for (User* user : auth->getUsers()) {
        if (user->getUsername() == username) {
            userRegistered = false;
            break;
        }
    }
    
    if (userRegistered) {
        QVERIFY(auth->registerUser(username, password));
    }
    
    // 2. Login with the user
    QVERIFY(auth->login(username, password));
    User* user = auth->getCurrentUser();
    QVERIFY(user != nullptr);
    QCOMPARE(user->getUsername(), username);
    
    // 3. Start a game and make moves
    gameLogic->resetBoard();
    QCOMPARE(gameLogic->getCurrentPlayer(), GameLogic::Player::X);
    
    // Make a move as the authenticated user
    QVERIFY(gameLogic->makeMove(0)); // X in top-left
    
    // Verify game state updated correctly
    QCOMPARE(gameLogic->getCellState(0), GameLogic::Player::X);
    QCOMPARE(gameLogic->getCurrentPlayer(), GameLogic::Player::O);
}

void TestIntegration::testGameCompletionToHistory()
{
    // 1. Register and login
    QString username = "historyplayer";
    QString password = "password123";
    
    // Make sure user doesn't already exist
    bool userRegistered = true;
    for (User* user : auth->getUsers()) {
        if (user->getUsername() == username) {
            userRegistered = false;
            break;
        }
    }
    
    if (userRegistered) {
        QVERIFY(auth->registerUser(username, password));
    }
    
    QVERIFY(auth->login(username, password));
    User* user = auth->getCurrentUser();
    
    // Record initial stats
    int initialTotalGames = user->getTotalGames();
    int initialWins = user->getWins();
    
    // 2. Play and complete a game
    gameLogic->resetBoard();
    
    // Make moves to create a win for X (player)
    gameLogic->makeMove(0); // X in top-left
    gameLogic->makeMove(3); // O in middle-left
    gameLogic->makeMove(1); // X in top-middle
    gameLogic->makeMove(4); // O in center
    gameLogic->makeMove(2); // X in top-right (win)
    
    // Verify game result
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
    
    // 3. Add game to user history
    user->addGame("win", "ai", "medium");
    
    // 4. Verify user statistics updated
    QCOMPARE(user->getTotalGames(), initialTotalGames + 1);
    QCOMPARE(user->getWins(), initialWins + 1);
    
    // 5. Verify game is in history
    QVector<GameRecord> history = user->getGameHistory();
    QVERIFY(!history.isEmpty());
    QVERIFY(history[0].result.contains("Win"));
}

void TestIntegration::testAIIntegration()
{
    // 1. Set up game with AI
    gameLogic->resetBoard();
    aiOpponent->setGameLogic(gameLogic);
    
    // 2. Player makes a move
    QVERIFY(gameLogic->makeMove(0)); // X in top-left
    
    // 3. AI responds
    aiOpponent->makeMove();
    waitForAIMove();
    
    // 4. Verify AI made a move
    int movesMade = 0;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) != GameLogic::Player::None) {
            movesMade++;
        }
    }
    QCOMPARE(movesMade, 2); // Player + AI moves
    
    // 5. Verify it's player's turn again
    QCOMPARE(gameLogic->getCurrentPlayer(), GameLogic::Player::X);
}

void TestIntegration::testDatabaseIntegration()
{
    // 1. Create test users
    User* testUser1 = new User("dbTestUser1", "hashedpass1");
    User* testUser2 = new User("dbTestUser2", "hashedpass2");
    
    // 2. Add some game data
    testUser1->addGame("win", "ai", "medium");
    testUser1->addGame("win", "dbTestUser2");
    testUser2->addGame("loss", "dbTestUser1");
    testUser2->addGame("win", "ai", "easy");
    
    // 3. Save to database
    QVector<User*> usersToSave;
    usersToSave.append(testUser1);
    usersToSave.append(testUser2);
    
    QVERIFY(database->saveUsers(usersToSave));
    
    // 4. Load from database
    QVector<User*> loadedUsers = database->loadUsers();
    QVERIFY(loadedUsers.size() >= 2);
    
    // 5. Generate and verify leaderboard
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(usersToSave);
    QVERIFY(leaderboard.size() >= 2);
    
    // Clean up
    for (User* user : loadedUsers) {
        delete user;
    }
    
    delete testUser1;
    delete testUser2;
}

QTEST_MAIN(TestIntegration)
#include "test_integration.moc" 