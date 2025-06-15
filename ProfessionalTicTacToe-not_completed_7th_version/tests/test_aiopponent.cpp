#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "aiopponent.h"
#include "gamelogic.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>
class AIOpponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        gameLogic = new GameLogic();
        aiOpponent = new AIOpponent();
        aiOpponent->setGameLogic(gameLogic);
    }

    void TearDown() override {
        delete aiOpponent;
        delete gameLogic;
    }

    GameLogic* gameLogic;
    AIOpponent* aiOpponent;
};

TEST_F(AIOpponentTest, SetGameLogic) {
    GameLogic* newGameLogic = new GameLogic();
    aiOpponent->setGameLogic(newGameLogic);

    // Test that AI can make moves with new game logic
    aiOpponent->makeMove();

    // Check that a move was made (at least one cell should be occupied)
    bool moveMade = false;
    for (int i = 0; i < 9; ++i) {
        if (newGameLogic->getCellState(i) != GameLogic::Player::None) {
            moveMade = true;
            break;
        }
    }
    EXPECT_TRUE(moveMade);

    delete newGameLogic;
}

TEST_F(AIOpponentTest, MakeValidMove) {
    // AI should make a valid move on empty board
    aiOpponent->makeMove();

    // Check that exactly one move was made
    int occupiedCells = 0;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) != GameLogic::Player::None) {
            occupiedCells++;
        }
    }
    EXPECT_EQ(occupiedCells, 1);
}

TEST_F(AIOpponentTest, WinningMoveDetection) {
    // Set up a winning scenario for O (AI)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(1); // O (AI will be O)
    gameLogic->makeMove(3); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(6); // X

    // Now it's O's turn and O can win at position 7
    aiOpponent->makeMove();

    // AI should have made the winning move
    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::OWins);
}

TEST_F(AIOpponentTest, BlockingMoveDetection) {
    // Set up a scenario where AI needs to block X from winning
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(4); // O (AI)
    gameLogic->makeMove(1); // X

    // X is about to win at position 2, AI should block
    aiOpponent->makeMove();

    // Check that AI blocked at position 2
    EXPECT_EQ(gameLogic->getCellState(2), GameLogic::Player::O);
}

TEST_F(AIOpponentTest, NoMovesOnFullBoard) {
    // Fill the entire board
    for (int i = 0; i < 9; ++i) {
        gameLogic->makeMove(i);
    }

    // AI should not crash when trying to make a move on full board
    aiOpponent->makeMove();

    // Game should be finished
    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::Draw);
}

TEST_F(AIOpponentTest, AIPlaysAsO) {
    // Make first move as X
    gameLogic->makeMove(0);

    // AI should make move as O
    aiOpponent->makeMove();

    // Check that AI placed O somewhere
    bool oFound = false;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) == GameLogic::Player::O) {
            oFound = true;
            break;
        }
    }
    EXPECT_TRUE(oFound);
}

TEST_F(AIOpponentTest, AIDoesNotMoveWhenGameFinished) {
    // Create a winning condition for X
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(3); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(2); // X wins

    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::XWins);

    // Count occupied cells before AI move
    int cellsBefore = 0;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) != GameLogic::Player::None) {
            cellsBefore++;
        }
    }

    // AI should not make a move when game is finished
    aiOpponent->makeMove();

    // Count occupied cells after AI move attempt
    int cellsAfter = 0;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) != GameLogic::Player::None) {
            cellsAfter++;
        }
    }

    // No new moves should have been made
    EXPECT_EQ(cellsBefore, cellsAfter);
}

