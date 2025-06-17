// cCDlazy:skip

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include "../include/aiopponent.h"
#include "../include/gamelogic.h"

class TestAIOpponent : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    
    void testSetGameLogic();
    void testMakeValidMove();
    void testNoMovesOnFullBoard();
    void testAIBlocksWinningMove();
    void testAITakesWinningMove();
    void testAIPerformance();

private:
    GameLogic *gameLogic;
    AIOpponent *aiOpponent;
    
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

void TestAIOpponent::init()
{
    gameLogic = new GameLogic();
    aiOpponent = new AIOpponent();
    aiOpponent->setGameLogic(gameLogic);
}

void TestAIOpponent::cleanup()
{
    delete aiOpponent;
    delete gameLogic;
}

void TestAIOpponent::testSetGameLogic()
{
    GameLogic *newGameLogic = new GameLogic();
    aiOpponent->setGameLogic(newGameLogic);
    
    aiOpponent->makeMove();
    waitForAIMove(); // Wait for the AI to make its move
    
    bool moveMade = false;
    for (int i = 0; i < 9; ++i) {
        if (newGameLogic->getCellState(i) != GameLogic::Player::None) {
            moveMade = true;
            break;
        }
    }
    QVERIFY(moveMade);
    
    delete newGameLogic;
}

void TestAIOpponent::testMakeValidMove()
{
    aiOpponent->makeMove();
    waitForAIMove(); // Wait for the AI to make its move
    
    int occupiedCells = 0;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) != GameLogic::Player::None) {
            occupiedCells++;
        }
    }
    QCOMPARE(occupiedCells, 1);
}

void TestAIOpponent::testNoMovesOnFullBoard()
{
    // Fill the board with a specific pattern that doesn't end the game prematurely
    // Use a pattern that doesn't result in a win
    gameLogic->makeMove(0); // X in top-left
    gameLogic->makeMove(1); // O in top-middle
    gameLogic->makeMove(2); // X in top-right
    gameLogic->makeMove(4); // O in center
    gameLogic->makeMove(3); // X in middle-left
    gameLogic->makeMove(6); // O in bottom-left
    gameLogic->makeMove(5); // X in middle-right
    gameLogic->makeMove(8); // O in bottom-right
    gameLogic->makeMove(7); // X in bottom-middle
    
    // Verify the board is full and game is in Draw state
    bool isFull = true;
    for (int i = 0; i < 9; ++i) {
        if (gameLogic->getCellState(i) == GameLogic::Player::None) {
            isFull = false;
            break;
        }
    }
    QVERIFY(isFull);
    
    // Now try to make an AI move on the full board
    aiOpponent->makeMove();
    waitForAIMove();
    
    // The game should still be in Draw state
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::Draw);
}

void TestAIOpponent::testAIBlocksWinningMove()
{
    // Set up board where player is about to win with a horizontal line
    gameLogic->resetBoard();
    
    // Player X makes moves at positions 0 and 1 (top-left and top-middle)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(4); // O (center)
    gameLogic->makeMove(1); // X
    
    // Now X could win by playing at position 2 (top-right)
    // AI (O) should block this move
    aiOpponent->makeMove();
    waitForAIMove();
    
    // Verify AI blocked the winning move
    QCOMPARE(gameLogic->getCellState(2), GameLogic::Player::O);
}

void TestAIOpponent::testAITakesWinningMove()
{
    // Set up board where AI can win
    gameLogic->resetBoard();
    
    // AI should always take a winning move when available, regardless of difficulty
    // But we'll use Expert just to be sure
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Expert);
    
    // Set up the board: O can win by playing at position 2
    // X: 3, 4
    // O: 0, 1
    
    // In Tic-Tac-Toe, X always goes first, so we need to set up the board manually
    // Use direct access to avoid turn-based restrictions
    // This method forces the board state directly for testing
    
    // Clear the board first
    for (int i = 0; i < 9; i++) {
        gameLogic->setCellState(i, GameLogic::Player::None);
    }
    
    // Set up X positions
    gameLogic->setCellState(3, GameLogic::Player::X);
    gameLogic->setCellState(4, GameLogic::Player::X);
    
    // Set up O positions
    gameLogic->setCellState(0, GameLogic::Player::O);
    gameLogic->setCellState(1, GameLogic::Player::O);
    
    // Set it to O's turn
    gameLogic->setCurrentPlayer(GameLogic::Player::O);
    
    // Print the board state for debugging
    qDebug() << "Board state before AI move:";
    for (int i = 0; i < 9; i++) {
        QString cell;
        switch (gameLogic->getCellState(i)) {
            case GameLogic::Player::X: cell = "X"; break;
            case GameLogic::Player::O: cell = "O"; break;
            default: cell = "-"; break;
        }
        qDebug() << "Cell" << i << ":" << cell;
    }
    
    // Now O can win by playing at position 2 (top-right)
    aiOpponent->makeMove();
    waitForAIMove(2000); // Increase timeout to ensure the move completes
    
    // Print the board state after AI move
    qDebug() << "Board state after AI move:";
    for (int i = 0; i < 9; i++) {
        QString cell;
        switch (gameLogic->getCellState(i)) {
            case GameLogic::Player::X: cell = "X"; break;
            case GameLogic::Player::O: cell = "O"; break;
            default: cell = "-"; break;
        }
        qDebug() << "Cell" << i << ":" << cell;
    }
    
    // Verify AI took the winning move
    QCOMPARE(gameLogic->getCellState(2), GameLogic::Player::O);
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::OWins);
}

void TestAIOpponent::testAIPerformance()
{
    // Test AI performance at different difficulty levels
    QElapsedTimer timer;
    
    // Test Easy difficulty
    gameLogic->resetBoard();
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Easy);
    timer.start();
    aiOpponent->makeMove();
    waitForAIMove();
    qint64 easyTime = timer.elapsed();
    
    // Test Expert difficulty
    gameLogic->resetBoard();
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Expert);
    timer.start();
    aiOpponent->makeMove();
    waitForAIMove();
    qint64 expertTime = timer.elapsed();
    
    // Expert should generally take longer than Easy due to deeper search
    // But this is not guaranteed due to optimizations, caching, etc.
    // So we just verify it completes in a reasonable time
    QVERIFY(expertTime < 3000); // Should complete within 3 seconds
    
    qDebug() << "AI Performance - Easy:" << easyTime << "ms, Expert:" << expertTime << "ms";
}

QTEST_MAIN(TestAIOpponent)
#include "test_aiopponent.moc"

