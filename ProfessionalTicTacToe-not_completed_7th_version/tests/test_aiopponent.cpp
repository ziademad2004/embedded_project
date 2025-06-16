#include <QtTest/QtTest>
#include "aiopponent.h"
#include "gamelogic.h"

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
    for (int i = 0; i < 9; ++i) {
        gameLogic->makeMove(i);
    }
    
    aiOpponent->makeMove();
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::Draw);
}

QTEST_MAIN(TestAIOpponent)

