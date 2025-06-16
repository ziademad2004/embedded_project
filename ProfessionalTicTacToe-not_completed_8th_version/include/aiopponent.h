#ifndef AIOPPONENT_H
#define AIOPPONENT_H

#include <QObject>
#include "gamelogic.h"
using Player = GameLogic::Player;

class AIOpponent : public QObject {
    Q_OBJECT

public:
    explicit AIOpponent(QObject *parent = nullptr);
    
    void setGameLogic(GameLogic *gameLogic);
    void makeMove();

private:
    GameLogic *m_gameLogic;
    
    // Constants for evaluation
    inline static constexpr int WIN_SCORE = 10;
    inline static constexpr int LOSE_SCORE = -10;
    inline static constexpr int DRAW_SCORE = 0;
    inline static constexpr int MAX_SCORE = 1000;
    
    // Minimax with alpha-beta pruning
    int minimax(QVector<Player>& board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth);
    
    // Helper functions
    int evaluateBoard(const QVector<Player>& board);
    int findBestMove(QVector<Player>& board, int maxDepth);
    bool isBoardFull(const QVector<Player>& board);
    bool checkWinner(const QVector<Player>& board, Player player);
};

#endif // AIOPPONENT_H
