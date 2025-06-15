#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "database.h"
#include "user.h"
#include "authentication.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>
class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        database = new Database();

        // Create test users with hashed passwords
        QString hashedPassword1 = Authentication::hashPassword("testPass1");
        QString hashedPassword2 = Authentication::hashPassword("testPass2");

        testUser1 = new User("testUser1", hashedPassword1);
        testUser2 = new User("testUser2", hashedPassword2);

        // Add some game data to test users
        testUser1->addGame("win", "ai", "medium");
        testUser1->addGame("loss", "testUser2");
        testUser1->addGame("draw", "ai", "hard");

        testUser2->addGame("win", "testUser1");
        testUser2->addGame("win", "ai", "easy");
    }

    void TearDown() override {
        delete testUser1;
        delete testUser2;
        delete database;

        // Clean up test database file
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString dbPath = appDataPath + "/tictactoe.json";
        QFile::remove(dbPath);
    }

    Database* database;
    User* testUser1;
    User* testUser2;
};

TEST_F(DatabaseTest, SaveSingleUser) {
    QVector<User*> users;
    users.append(testUser1);

    EXPECT_TRUE(database->saveUsers(users));
}

TEST_F(DatabaseTest, SaveMultipleUsers) {
    QVector<User*> usersToSave;
    usersToSave.append(testUser1);
    usersToSave.append(testUser2);

    EXPECT_TRUE(database->saveUsers(usersToSave));

    // Verify both users were saved by loading
    QVector<User*> loadedUsers = database->loadUsers();
    EXPECT_GE(loadedUsers.size(), 2);

    // Cleanup loaded users
    for (User* user : loadedUsers) {
        delete user;
    }
}

TEST_F(DatabaseTest, LoadUsers) {
    // Save users first
    QVector<User*> usersToSave;
    usersToSave.append(testUser1);
    usersToSave.append(testUser2);
    database->saveUsers(usersToSave);

    // Load users
    QVector<User*> loadedUsers = database->loadUsers();
    EXPECT_GE(loadedUsers.size(), 2);

    // Verify user data
    bool user1Found = false, user2Found = false;
    for (User* user : loadedUsers) {
        if (user->getUsername() == testUser1->getUsername()) {
            user1Found = true;
            EXPECT_EQ(user->getTotalGames(), testUser1->getTotalGames());
        }
        if (user->getUsername() == testUser2->getUsername()) {
            user2Found = true;
            EXPECT_EQ(user->getTotalGames(), testUser2->getTotalGames());
        }
    }
    EXPECT_TRUE(user1Found);
    EXPECT_TRUE(user2Found);

    // Cleanup
    for (User* user : loadedUsers) {
        delete user;
    }
}

TEST_F(DatabaseTest, LoadEmptyDatabase) {
    // Load from empty/non-existent database
    QVector<User*> loadedUsers = database->loadUsers();

    // Should return default users or empty list without crashing
    EXPECT_GE(loadedUsers.size(), 0);

    // Cleanup
    for (User* user : loadedUsers) {
        delete user;
    }
}

TEST_F(DatabaseTest, SaveGameRecord) {
    QString playerX = "player1";
    QString playerO = "player2";
    QString result = "X wins";

    EXPECT_TRUE(database->saveGame(playerX, playerO, result));
}

TEST_F(DatabaseTest, LeaderboardGeneration) {
    QVector<User*> users;
    users.append(testUser1);
    users.append(testUser2);

    // Generate leaderboard
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);

    EXPECT_GE(leaderboard.size(), 2);

    // Verify leaderboard entries have correct data
    for (const LeaderboardEntry& entry : leaderboard) {
        EXPECT_FALSE(entry.username.isEmpty());
        EXPECT_GE(entry.wins, 0);
        EXPECT_GE(entry.totalGames, 0);
        EXPECT_GE(entry.winRate, 0);
        EXPECT_LE(entry.winRate, 100);
        EXPECT_GE(entry.bestStreak, 0);
        EXPECT_GE(entry.score, 0);
    }
}

TEST_F(DatabaseTest, LeaderboardSorting) {
    // Create users with different scores
    QString hashedPassword = Authentication::hashPassword("testPass");
    User* highScoreUser = new User("highScore", hashedPassword);
    User* lowScoreUser = new User("lowScore", hashedPassword);

    // High score user: many wins
    for (int i = 0; i < 10; ++i) {
        highScoreUser->addGame("win", "ai", "hard");
    }

    // Low score user: few wins
    lowScoreUser->addGame("win", "ai", "easy");
    lowScoreUser->addGame("loss", "ai", "easy");

    QVector<User*> users;
    users.append(lowScoreUser);  // Add low score first
    users.append(highScoreUser); // Add high score second

    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);

    EXPECT_EQ(leaderboard.size(), 2);

    // High score user should be first (leaderboard should be sorted)
    EXPECT_EQ(leaderboard[0].username.toStdString(), "highScore");
    EXPECT_GT(leaderboard[0].score, leaderboard[1].score);

    // Cleanup
    delete highScoreUser;
    delete lowScoreUser;
}

TEST_F(DatabaseTest, UserStatsPersistence) {
    // Save user with specific stats
    QVector<User*> users;
    users.append(testUser1);
    database->saveUsers(users);

    // Load users and verify stats are preserved
    QVector<User*> loadedUsers = database->loadUsers();

    bool userFound = false;
    for (User* user : loadedUsers) {
        if (user->getUsername() == testUser1->getUsername()) {
            userFound = true;
            EXPECT_EQ(user->getTotalGames(), testUser1->getTotalGames());
            EXPECT_EQ(user->getWins(), testUser1->getWins());
            EXPECT_EQ(user->getLosses(), testUser1->getLosses());
            EXPECT_EQ(user->getDraws(), testUser1->getDraws());
            EXPECT_EQ(user->getVsAI(), testUser1->getVsAI());
            EXPECT_EQ(user->getVsPlayers(), testUser1->getVsPlayers());
            break;
        }
    }
    EXPECT_TRUE(userFound);

    // Cleanup
    for (User* user : loadedUsers) {
        delete user;
    }
}

