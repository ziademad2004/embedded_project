// clazy:skip

#include <QTest>
#include <QObject>
#include <QTemporaryFile>
#include <QDir>
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
    void testErrorHandling();
    void testLeaderboardSorting();
    void testLargeDataset();

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

void TestDatabase::testErrorHandling()
{
    // Test with invalid file path
    Database invalidDb;
    
    // Use a path that should not be writable
    QString invalidPath = "/invalid/path/to/file.json";
    
    // Use reflection to set the private member (this is just for testing)
    invalidDb.setProperty("m_dbPath", invalidPath);
    
    QVector<User*> users;
    users.append(new User("erroruser", "password"));
    
    // This should fail because the path is invalid
    QVERIFY(!invalidDb.saveUsers(users));
    
    // Test with empty path
    invalidDb.setProperty("m_dbPath", "");
    QVERIFY(!invalidDb.saveUsers(users));
    
    // Test with invalid characters
    invalidDb.setProperty("m_dbPath", "//invalid//path.json");
    QVERIFY(!invalidDb.saveUsers(users));
    
    // Clean up
    for (User* user : users) {
        delete user;
    }
}

void TestDatabase::testLeaderboardSorting()
{
    // Create users with different stats
    User* user1 = new User("user1", "pass");
    User* user2 = new User("user2", "pass");
    User* user3 = new User("user3", "pass");
    
    // Add different game outcomes
    for (int i = 0; i < 5; i++) user1->addGame("win", "ai");
    for (int i = 0; i < 3; i++) user2->addGame("win", "ai");
    for (int i = 0; i < 10; i++) user3->addGame("win", "ai");
    
    QVector<User*> users = {user1, user2, user3};
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);
    
    // Verify sorting (highest score first)
    QVERIFY(leaderboard.size() >= 3);
    
    // Find the entries for our test users
    int user1Score = -1, user2Score = -1, user3Score = -1;
    for (const LeaderboardEntry& entry : leaderboard) {
        if (entry.username == "user1") user1Score = entry.score;
        if (entry.username == "user2") user2Score = entry.score;
        if (entry.username == "user3") user3Score = entry.score;
    }
    
    // Verify user3 (10 wins) has higher score than user1 (5 wins)
    // And user1 has higher score than user2 (3 wins)
    QVERIFY(user3Score > user1Score);
    QVERIFY(user1Score > user2Score);
    
    // Clean up
    delete user1;
    delete user2;
    delete user3;
}

void TestDatabase::testLargeDataset()
{
    // Create a large number of users
    QVector<User*> manyUsers;
    const int userCount = 100;
    
    for (int i = 0; i < userCount; i++) {
        QString username = QString("user%1").arg(i);
        User* user = new User(username, "password");
        
        // Add some random game data
        int wins = i % 10; // 0-9 wins
        for (int j = 0; j < wins; j++) {
            user->addGame("win", "ai");
        }
        
        // Add more data to make the dataset larger
        for (int j = 0; j < i % 5; j++) {
            user->addGame("loss", "ai", "expert");
        }
        
        for (int j = 0; j < i % 3; j++) {
            user->addGame("draw", QString("opponent%1").arg(j));
        }
        
        manyUsers.append(user);
    }
    
    // Make sure the database has a valid path for this test
    QFileInfo fileInfo(database->property("m_dbPath").toString());
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Test saving many users
    QVERIFY(database->saveUsers(manyUsers));
    
    // Test loading many users
    QVector<User*> loadedUsers = database->loadUsers();
    QVERIFY(loadedUsers.size() >= 2); // At least the default users should be loaded
    
    // Test generating leaderboard with many users
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(manyUsers);
    QVERIFY(leaderboard.size() >= 2); // At least some users should be in the leaderboard
    
    // Clean up
    for (User* user : manyUsers) {
        delete user;
    }
    
    for (User* user : loadedUsers) {
        delete user;
    }
}

QTEST_MAIN(TestDatabase)
#include "test_database.moc"

