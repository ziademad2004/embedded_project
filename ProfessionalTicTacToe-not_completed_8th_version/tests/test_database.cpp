// clazy:skip

#include <QTest>
#include <QObject>
#include "../include/database.h"
#include "../include/user.h"
#include "../include/authentication.h"

class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    
    void testSaveAndLoadUsers();
    void testLeaderboardGeneration();
    void testSaveGame();

private:
    Database *database;
    User *testUser1;
    User *testUser2;
};

void TestDatabase::init()
{
    database = new Database();
    
    QString hashedPassword1 = Authentication::hashPassword("testPass1");
    QString hashedPassword2 = Authentication::hashPassword("testPass2");
    
    testUser1 = new User("testUser1", hashedPassword1);
    testUser2 = new User("testUser2", hashedPassword2);
    
    testUser1->addGame("win", "ai", "medium");
    testUser1->addGame("loss", "testUser2");
    
    testUser2->addGame("win", "testUser1");
    testUser2->addGame("win", "ai", "easy");
}

void TestDatabase::cleanup()
{
    delete testUser1;
    delete testUser2;
    delete database;
}

void TestDatabase::testSaveAndLoadUsers()
{
    QVector<User*> usersToSave;
    usersToSave.append(testUser1);
    usersToSave.append(testUser2);
    
    QVERIFY(database->saveUsers(usersToSave));
    
    QVector<User*> loadedUsers = database->loadUsers();
    QVERIFY(loadedUsers.size() >= 2);
    
    for (User* user : loadedUsers) {
        delete user;
    }
}

void TestDatabase::testLeaderboardGeneration()
{
    QVector<User*> users;
    users.append(testUser1);
    users.append(testUser2);
    
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);
    QVERIFY(leaderboard.size() >= 2);
    
    for (const LeaderboardEntry& entry : leaderboard) {
        QVERIFY(!entry.username.isEmpty());
        QVERIFY(entry.wins >= 0);
        QVERIFY(entry.totalGames >= 0);
        QVERIFY(entry.winRate >= 0 && entry.winRate <= 100);
        QVERIFY(entry.bestStreak >= 0);
        QVERIFY(entry.score >= 0);
    }
}

void TestDatabase::testSaveGame()
{
    QString playerX = "player1";
    QString playerO = "player2";
    QString result = "X wins";
    
    QVERIFY(database->saveGame(playerX, playerO, result));
}

QTEST_MAIN(TestDatabase)
#include "test_database.moc"

