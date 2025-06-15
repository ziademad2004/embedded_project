#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "authentication.h"
#include "gamelogic.h"
#include "aiopponent.h"
#include "database.h"
#include "gamehistory.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        auth = new Authentication();
        gameLogic = new GameLogic();
        aiOpponent = new AIOpponent();
        database = new Database();
        gameHistory = new GameHistory();

        aiOpponent->setGameLogic(gameLogic);
    }

    void TearDown() override {
        delete gameHistory;
        delete database;
        delete aiOpponent;
        delete gameLogic;
        delete auth;
    }

    Authentication* auth;
    GameLogic* gameLogic;
    AIOpponent* aiOpponent;
    Database* database;
    GameHistory* gameHistory;
};

TEST_F(IntegrationTest, CompleteGameFlow) {
    // Register and login user
    QString username = "testPlayer";
    QString password = "testPass123";

    EXPECT_TRUE(auth->registerUser(username, password));
    EXPECT_TRUE(auth->login(username, password));

    User* currentUser = auth->getCurrentUser();
    ASSERT_NE(currentUser, nullptr);

    // Play a complete game
    EXPECT_TRUE(gameLogic->makeMove(0)); // X

    // Let AI make a move
    aiOpponent->makeMove(); // O

    // Continue game until finished
    int moveCount = 2;
    while (gameLogic->getGameResult() == GameLogic::GameResult::InProgress && moveCount < 9) {
        // Find next available move for X
        bool moveMade = false;
        for (int i = 0; i < 9 && !moveMade; ++i) {
            if (gameLogic->getCellState(i) == GameLogic::Player::None) {
                gameLogic->makeMove(i);
                moveMade = true;
                moveCount++;
            }
        }

        if (gameLogic->getGameResult() == GameLogic::GameResult::InProgress) {
            aiOpponent->makeMove();
            moveCount++;
        }
    }

    // Game should be finished
    EXPECT_NE(gameLogic->getGameResult(), GameLogic::GameResult::InProgress);

    // Update user statistics based on result
    QString result;
    if (gameLogic->getGameResult() == GameLogic::GameResult::XWins) {
        result = "win";
    } else if (gameLogic->getGameResult() == GameLogic::GameResult::OWins) {
        result = "loss";
    } else {
        result = "draw";
    }

    int initialGames = currentUser->getTotalGames();
    currentUser->addGame(result, "ai", "medium");

    EXPECT_EQ(currentUser->getTotalGames(), initialGames + 1);
}

TEST_F(IntegrationTest, DatabaseUserPersistence) {
    QString username = "dbTestUser";
    QString password = "dbTestPass";

    // Register user
    EXPECT_TRUE(auth->registerUser(username, password));

    // Save users to database
    QVector<User*> users = auth->getUsers();
    EXPECT_TRUE(database->saveUsers(users));

    // Load users from database
    QVector<User*> loadedUsers = database->loadUsers();
    EXPECT_GT(loadedUsers.size(), 0);

    // Find our test user
    bool userFound = false;
    for (User* user : loadedUsers) {
        if (user->getUsername() == username) {
            userFound = true;
            break;
        }
    }
    EXPECT_TRUE(userFound);

    // Cleanup
    for (User* user : loadedUsers) {
        delete user;
    }
}

TEST_F(IntegrationTest, PlayerVsPlayerAuthentication) {
    QString player1 = "player1";
    QString password1 = "pass1";
    QString player2 = "player2";
    QString password2 = "pass2";
    QString errorMessage;

    // Register both players
    EXPECT_TRUE(auth->registerUser(player1, password1));
    EXPECT_TRUE(auth->registerUser(player2, password2));

    // Authenticate for PvP game
    EXPECT_TRUE(auth->authenticatePlayers(player1, password1, player2, password2, errorMessage));
    EXPECT_TRUE(errorMessage.isEmpty());

    // Test with wrong credentials
    EXPECT_FALSE(auth->authenticatePlayers(player1, "wrong", player2, password2, errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST_F(IntegrationTest, GameHistoryIntegration) {
    // Add entries to game history
    gameHistory->addEntry("Player1 vs Player2: Player1 wins");
    gameHistory->addEntry("Player1 vs AI: Draw");
    gameHistory->addEntry("Player2 vs AI: AI wins");

    // Get entries
    QVector<GameHistory::HistoryEntry> entries = gameHistory->getEntries();
    EXPECT_EQ(entries.size(), 3);

    // Verify entries are in correct order (newest first)
    EXPECT_TRUE(entries[0].result.contains("Player2 vs AI"));
    EXPECT_TRUE(entries[1].result.contains("Player1 vs AI"));
    EXPECT_TRUE(entries[2].result.contains("Player1 vs Player2"));
}

TEST_F(IntegrationTest, AIGameLogicIntegration) {
    // Test that AI and GameLogic work together properly
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Hard);
    EXPECT_EQ(gameLogic->getAIDifficulty(), GameLogic::AIDifficulty::Hard);

    // Make a move and let AI respond
    EXPECT_TRUE(gameLogic->makeMove(4)); // X takes center
    aiOpponent->makeMove(); // AI responds

    // Verify AI made a move
    int occupiedCells = 0;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) != GameLogic::Player::None) {
            occupiedCells++;
        }
    }
    EXPECT_EQ(occupiedCells, 2); // X and O moves
}

TEST_F(IntegrationTest, FullSystemWorkflow) {
    // Complete workflow test
    QString username = "fullTestUser";
    QString password = "fullTestPass";

    // 1. User registration and authentication
    EXPECT_TRUE(auth->registerUser(username, password));
    EXPECT_TRUE(auth->login(username, password));

    // 2. Game setup and play
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Medium);
    gameLogic->makeMove(0); // User move
    aiOpponent->makeMove(); // AI move

    // 3. Game history tracking
    gameHistory->addEntry(QString("%1 vs AI: Game in progress").arg(username));

    // 4. User statistics update
    User* user = auth->getCurrentUser();
    user->addGame("win", "ai", "medium");

    // 5. Database persistence
    QVector<User*> users = auth->getUsers();
    EXPECT_TRUE(database->saveUsers(users));

    // 6. Leaderboard generation
    QVector<LeaderboardEntry> leaderboard = database->getLeaderboard(users);
    EXPECT_GT(leaderboard.size(), 0);

    // Verify everything worked
    EXPECT_EQ(user->getTotalGames(), 1);
    EXPECT_EQ(user->getWins(), 1);
    EXPECT_GT(gameHistory->getEntries().size(), 0);
}

