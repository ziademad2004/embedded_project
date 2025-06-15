#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "gamelogic.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>
class GameLogicTest : public ::testing::Test {
protected:
    void SetUp() override {
        gameLogic = new GameLogic();
    }

    void TearDown() override {
        delete gameLogic;
    }

    GameLogic* gameLogic;
};

TEST_F(GameLogicTest, InitialGameState) {
    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::InProgress);
    EXPECT_EQ(gameLogic->getCurrentPlayer(), GameLogic::Player::X);

    // Check all cells are empty
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(gameLogic->getCellState(i), GameLogic::Player::None);
    }
}

TEST_F(GameLogicTest, ValidMove) {
    EXPECT_TRUE(gameLogic->makeMove(0));
    EXPECT_EQ(gameLogic->getCellState(0), GameLogic::Player::X);
    EXPECT_EQ(gameLogic->getCurrentPlayer(), GameLogic::Player::O);
}

TEST_F(GameLogicTest, InvalidMoveOccupiedCell) {
    gameLogic->makeMove(0);
    EXPECT_FALSE(gameLogic->makeMove(0)); // Same cell
    EXPECT_EQ(gameLogic->getCurrentPlayer(), GameLogic::Player::O);
}

TEST_F(GameLogicTest, InvalidMoveOutOfBounds) {
    EXPECT_FALSE(gameLogic->makeMove(-1));
    EXPECT_FALSE(gameLogic->makeMove(9));
    EXPECT_FALSE(gameLogic->makeMove(10));
}

TEST_F(GameLogicTest, WinConditionHorizontal) {
    // X wins first row (positions 0, 1, 2)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(3); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(2); // X wins

    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
}

TEST_F(GameLogicTest, WinConditionVertical) {
    // X wins first column (positions 0, 3, 6)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(1); // O
    gameLogic->makeMove(3); // X
    gameLogic->makeMove(2); // O
    gameLogic->makeMove(6); // X wins

    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
}

TEST_F(GameLogicTest, WinConditionDiagonal) {
    // X wins main diagonal (positions 0, 4, 8)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(1); // O
    gameLogic->makeMove(4); // X
    gameLogic->makeMove(2); // O
    gameLogic->makeMove(8); // X wins

    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
}

TEST_F(GameLogicTest, DrawCondition) {
    // Create a draw scenario
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(1); // O
    gameLogic->makeMove(2); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(3); // X
    gameLogic->makeMove(6); // O
    gameLogic->makeMove(5); // X
    gameLogic->makeMove(8); // O
    gameLogic->makeMove(7); // X - Draw

    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::Draw);
}

TEST_F(GameLogicTest, ResetBoard) {
    gameLogic->makeMove(0);
    gameLogic->makeMove(4);
    gameLogic->resetBoard();

    EXPECT_EQ(gameLogic->getGameResult(), GameLogic::GameResult::InProgress);
    EXPECT_EQ(gameLogic->getCurrentPlayer(), GameLogic::Player::X);

    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(gameLogic->getCellState(i), GameLogic::Player::None);
    }
}

TEST_F(GameLogicTest, CheckWinner) {
    // Test checkWinner method
    EXPECT_FALSE(gameLogic->checkWinner());

    // Create winning condition
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(3); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(2); // X wins

    EXPECT_TRUE(gameLogic->checkWinner());
}

TEST_F(GameLogicTest, AIDifficultySettings) {
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Easy);
    EXPECT_EQ(gameLogic->getAIDifficulty(), GameLogic::AIDifficulty::Easy);

    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Hard);
    EXPECT_EQ(gameLogic->getAIDifficulty(), GameLogic::AIDifficulty::Hard);

    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Expert);
    EXPECT_EQ(gameLogic->getAIDifficulty(), GameLogic::AIDifficulty::Expert);
}

TEST_F(GameLogicTest, WinPattern) {
    // Test that win pattern is initially empty
    QVector<int> pattern = gameLogic->getWinPattern();
    EXPECT_TRUE(pattern.isEmpty());

    // Create a winning condition and check pattern
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(3); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(2); // X wins

    pattern = gameLogic->getWinPattern();
    EXPECT_FALSE(pattern.isEmpty());
    EXPECT_EQ(pattern.size(), 3);
}


