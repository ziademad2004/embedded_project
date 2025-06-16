// clazy:skip

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
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

QTEST_MAIN(TestAIOpponent)
#include "test_aiopponent.moc"

