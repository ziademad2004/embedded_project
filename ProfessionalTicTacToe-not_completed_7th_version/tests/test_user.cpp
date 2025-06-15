#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "user.h"
#include "authentication.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>
class UserTest : public ::testing::Test {
protected:
    void SetUp() override {
        QString hashedPassword = Authentication::hashPassword("testPassword");
        user = new User("testUser", hashedPassword);
    }

    void TearDown() override {
        delete user;
    }

    User* user;
};

TEST_F(UserTest, InitialStats) {
    EXPECT_EQ(user->getUsername().toStdString(), "testUser");
    EXPECT_EQ(user->getTotalGames(), 0);
    EXPECT_EQ(user->getWins(), 0);
    EXPECT_EQ(user->getLosses(), 0);
    EXPECT_EQ(user->getDraws(), 0);
    EXPECT_EQ(user->getVsAI(), 0);
    EXPECT_EQ(user->getVsPlayers(), 0);
    EXPECT_EQ(user->getWinRate(), 0);
    EXPECT_EQ(user->getBestStreak(), 0);
}

TEST_F(UserTest, PasswordCheck) {
    EXPECT_TRUE(user->checkPassword("testPassword"));
    EXPECT_FALSE(user->checkPassword("wrongPassword"));
    EXPECT_FALSE(user->checkPassword(""));
}

TEST_F(UserTest, HashedPasswordAccess) {
    QString hashedPassword = user->getHashedPassword();
    EXPECT_FALSE(hashedPassword.isEmpty());
    EXPECT_NE(hashedPassword.toStdString(), "testPassword"); // Should not be plain text
}

TEST_F(UserTest, AddWinGame) {
    user->addGame("win", "ai", "medium");

    EXPECT_EQ(user->getTotalGames(), 1);
    EXPECT_EQ(user->getWins(), 1);
    EXPECT_EQ(user->getLosses(), 0);
    EXPECT_EQ(user->getDraws(), 0);
    EXPECT_EQ(user->getVsAI(), 1);
    EXPECT_EQ(user->getVsPlayers(), 0);
    EXPECT_EQ(user->getWinRate(), 100);
    EXPECT_EQ(user->getBestStreak(), 1);
}

TEST_F(UserTest, AddLossGame) {
    user->addGame("loss", "player2");

    EXPECT_EQ(user->getTotalGames(), 1);
    EXPECT_EQ(user->getWins(), 0);
    EXPECT_EQ(user->getLosses(), 1);
    EXPECT_EQ(user->getDraws(), 0);
    EXPECT_EQ(user->getVsAI(), 0);
    EXPECT_EQ(user->getVsPlayers(), 1);
    EXPECT_EQ(user->getWinRate(), 0);
    EXPECT_EQ(user->getBestStreak(), 0);
}

TEST_F(UserTest, AddDrawGame) {
    user->addGame("draw", "ai", "easy");

    EXPECT_EQ(user->getTotalGames(), 1);
    EXPECT_EQ(user->getWins(), 0);
    EXPECT_EQ(user->getLosses(), 0);
    EXPECT_EQ(user->getDraws(), 1);
    EXPECT_EQ(user->getVsAI(), 1);
    EXPECT_EQ(user->getVsPlayers(), 0);
    EXPECT_EQ(user->getWinRate(), 0);
    EXPECT_EQ(user->getBestStreak(), 0);
}

TEST_F(UserTest, WinStreakCalculation) {
    user->addGame("win", "ai");
    user->addGame("win", "ai");
    user->addGame("win", "ai");
    user->addGame("loss", "ai");
    user->addGame("win", "ai");
    user->addGame("win", "ai");

    EXPECT_EQ(user->getBestStreak(), 3);
    EXPECT_EQ(user->getTotalGames(), 6);
    EXPECT_EQ(user->getWins(), 5);
    EXPECT_EQ(user->getLosses(), 1);
}

TEST_F(UserTest, GameHistory) {
    user->addGame("win", "ai", "medium");
    user->addGame("loss", "player2");

    QVector<GameRecord> history = user->getGameHistory();
    EXPECT_EQ(history.size(), 2);

    // History should be in reverse chronological order (newest first)
    EXPECT_TRUE(history[0].result.contains("player2"));
    EXPECT_TRUE(history[1].result.contains("AI"));
}

TEST_F(UserTest, AddGameWithDate) {
    QString customDate = "2025-06-15 20:30:00";
    user->addGameWithDate("win", "ai", customDate, "hard");

    EXPECT_EQ(user->getTotalGames(), 1);
    EXPECT_EQ(user->getWins(), 1);

    QVector<GameRecord> history = user->getGameHistory();
    EXPECT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].date.toStdString(), customDate.toStdString());
}

TEST_F(UserTest, DefaultConstructor) {
    User defaultUser;
    EXPECT_TRUE(defaultUser.getUsername().isEmpty());
    EXPECT_EQ(defaultUser.getTotalGames(), 0);
    EXPECT_EQ(defaultUser.getWins(), 0);
    EXPECT_EQ(defaultUser.getLosses(), 0);
    EXPECT_EQ(defaultUser.getDraws(), 0);
}
