// clazy:skip

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include "../include/gamelogic.h"

class TestGameLogic : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testInitialState();
    void testValidMove();
    void testInvalidMove();
    void testWinConditionHorizontal();
    void testWinConditionVertical();
    void testWinConditionDiagonal();
    void testDrawCondition();
    void testResetBoard();
    void testAIDifficulty();
    void testSignalsEmitted();

private:
    GameLogic *gameLogic;
};

void TestGameLogic::initTestCase()
{
    // Called before the first test function is executed
}

void TestGameLogic::cleanupTestCase()
{
    // Called after the last test function was executed
}

void TestGameLogic::init()
{
    // Called before each test function is executed
    gameLogic = new GameLogic();
}

void TestGameLogic::cleanup()
{
    // Called after every test function
    delete gameLogic;
}

void TestGameLogic::testInitialState()
{
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::InProgress);
    QCOMPARE(gameLogic->getCurrentPlayer(), GameLogic::Player::X);
    
    for (int i = 0; i < 9; ++i) {
        QCOMPARE(gameLogic->getCellState(i), GameLogic::Player::None);
    }
}

void TestGameLogic::testValidMove()
{
    QVERIFY(gameLogic->makeMove(0));
    QCOMPARE(gameLogic->getCellState(0), GameLogic::Player::X);
    QCOMPARE(gameLogic->getCurrentPlayer(), GameLogic::Player::O);
}

void TestGameLogic::testInvalidMove()
{
    gameLogic->makeMove(0);
    QVERIFY(!gameLogic->makeMove(0)); // Same cell
    QVERIFY(!gameLogic->makeMove(-1)); // Out of bounds
    QVERIFY(!gameLogic->makeMove(9)); // Out of bounds
}

void TestGameLogic::testWinConditionHorizontal()
{
    // X wins first row (0, 1, 2)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(3); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(4); // O
    gameLogic->makeMove(2); // X wins
    
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
}

void TestGameLogic::testWinConditionVertical()
{
    // X wins first column (0, 3, 6)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(1); // O
    gameLogic->makeMove(3); // X
    gameLogic->makeMove(2); // O
    gameLogic->makeMove(6); // X wins
    
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
}

void TestGameLogic::testWinConditionDiagonal()
{
    // X wins main diagonal (0, 4, 8)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(1); // O
    gameLogic->makeMove(4); // X
    gameLogic->makeMove(2); // O
    gameLogic->makeMove(8); // X wins
    
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::XWins);
}

void TestGameLogic::testDrawCondition()
{
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
    
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::Draw);
}

void TestGameLogic::testResetBoard()
{
    gameLogic->makeMove(0);
    gameLogic->makeMove(4);
    gameLogic->resetBoard();
    
    QCOMPARE(gameLogic->getGameResult(), GameLogic::GameResult::InProgress);
    QCOMPARE(gameLogic->getCurrentPlayer(), GameLogic::Player::X);
    
    for (int i = 0; i < 9; ++i) {
        QCOMPARE(gameLogic->getCellState(i), GameLogic::Player::None);
    }
}

void TestGameLogic::testAIDifficulty()
{
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Easy);
    QCOMPARE(gameLogic->getAIDifficulty(), GameLogic::AIDifficulty::Easy);
    
    gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Hard);
    QCOMPARE(gameLogic->getAIDifficulty(), GameLogic::AIDifficulty::Hard);
}

void TestGameLogic::testSignalsEmitted()
{
    QSignalSpy boardChangedSpy(gameLogic, SIGNAL(boardChanged()));
    QSignalSpy playerChangedSpy(gameLogic, SIGNAL(playerChanged(GameLogic::Player)));
    
    // Make a move
    gameLogic->makeMove(0);
    
    // Verify signals were emitted
    QCOMPARE(boardChangedSpy.count(), 1);
    QCOMPARE(playerChangedSpy.count(), 1);
    
    // Make another move
    gameLogic->makeMove(1);
    
    // Verify signals were emitted again
    QCOMPARE(boardChangedSpy.count(), 2);
    QCOMPARE(playerChangedSpy.count(), 2);
    
    // Set up a winning move
    gameLogic->resetBoard();
    boardChangedSpy.clear();
    playerChangedSpy.clear();
    QSignalSpy gameOverSpy(gameLogic, SIGNAL(gameOver(GameLogic::GameResult)));
    
    // Make moves for a win (X: 0,1,2)
    gameLogic->makeMove(0); // X
    gameLogic->makeMove(3); // O
    gameLogic->makeMove(1); // X
    gameLogic->makeMove(4); // O
    
    // Winning move
    gameLogic->makeMove(2); // X wins
    
    // Verify game over signal was emitted
    QCOMPARE(gameOverSpy.count(), 1);
    
    // Verify the result is correct
    QList<QVariant> arguments = gameOverSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<GameLogic::GameResult>(), GameLogic::GameResult::XWins);
}

QTEST_MAIN(TestGameLogic)
#include "test_gamelogic.moc"

