// clazy:skip

#include <QTest>
#include <QObject>
#include "../include/user.h"
#include "../include/authentication.h"

class TestUser : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    
    void testInitialStats();
    void testPasswordCheck();
    void testAddWinGame();
    void testAddLossGame();
    void testAddDrawGame();
    void testWinStreakCalculation();
    void testGameHistory();

private:
    User *user;
};

void TestUser::init()
{
    QString hashedPassword = Authentication::hashPassword("testPassword");
    user = new User("testUser", hashedPassword);
}

void TestUser::cleanup()
{
    delete user;
}

void TestUser::testInitialStats()
{
    QCOMPARE(user->getUsername(), QString("testUser"));
    QCOMPARE(user->getTotalGames(), 0);
    QCOMPARE(user->getWins(), 0);
    QCOMPARE(user->getLosses(), 0);
    QCOMPARE(user->getDraws(), 0);
    QCOMPARE(user->getVsAI(), 0);
    QCOMPARE(user->getVsPlayers(), 0);
    QCOMPARE(user->getWinRate(), 0);
    QCOMPARE(user->getBestStreak(), 0);
}

void TestUser::testPasswordCheck()
{
    QVERIFY(user->checkPassword("testPassword"));
    QVERIFY(!user->checkPassword("wrongPassword"));
    QVERIFY(!user->checkPassword(""));
}

void TestUser::testAddWinGame()
{
    user->addGame("win", "ai", "medium");
    
    QCOMPARE(user->getTotalGames(), 1);
    QCOMPARE(user->getWins(), 1);
    QCOMPARE(user->getLosses(), 0);
    QCOMPARE(user->getDraws(), 0);
    QCOMPARE(user->getVsAI(), 1);
    QCOMPARE(user->getVsPlayers(), 0);
    QCOMPARE(user->getWinRate(), 100);
    QCOMPARE(user->getBestStreak(), 1);
}

void TestUser::testAddLossGame()
{
    user->addGame("loss", "player2");
    
    QCOMPARE(user->getTotalGames(), 1);
    QCOMPARE(user->getWins(), 0);
    QCOMPARE(user->getLosses(), 1);
    QCOMPARE(user->getDraws(), 0);
    QCOMPARE(user->getVsAI(), 0);
    QCOMPARE(user->getVsPlayers(), 1);
    QCOMPARE(user->getWinRate(), 0);
    QCOMPARE(user->getBestStreak(), 0);
}

void TestUser::testAddDrawGame()
{
    user->addGame("draw", "ai", "easy");
    
    QCOMPARE(user->getTotalGames(), 1);
    QCOMPARE(user->getWins(), 0);
    QCOMPARE(user->getLosses(), 0);
    QCOMPARE(user->getDraws(), 1);
    QCOMPARE(user->getVsAI(), 1);
    QCOMPARE(user->getVsPlayers(), 0);
    QCOMPARE(user->getWinRate(), 0);
    QCOMPARE(user->getBestStreak(), 0);
}

void TestUser::testWinStreakCalculation()
{
    user->addGame("win", "ai");
    user->addGame("win", "ai");
    user->addGame("win", "ai");
    user->addGame("loss", "ai");
    user->addGame("win", "ai");
    user->addGame("win", "ai");
    
    QCOMPARE(user->getBestStreak(), 3);
    QCOMPARE(user->getTotalGames(), 6);
    QCOMPARE(user->getWins(), 5);
    QCOMPARE(user->getLosses(), 1);
}

void TestUser::testGameHistory()
{
    user->addGame("win", "ai", "medium");
    user->addGame("loss", "player2");
    
    QVector<GameRecord> history = user->getGameHistory();
    QCOMPARE(history.size(), 2);
    
    // Game history is stored in reverse order (newest first)
    // Checking the most recent game first (index 0)
    QVERIFY(history[0].result.contains("player2"));
    
    // Checking the older game (index 1)
    QVERIFY(history[1].result.contains("AI"));
}

QTEST_MAIN(TestUser)
#include "test_user.moc"
