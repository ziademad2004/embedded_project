#ifndef AIOPPONENT_H
#define AIOPPONENT_H

#include <QObject>
#include <QVector>
#include "gamelogic.h"

class AIOpponent : public QObject
{
    Q_OBJECT

public:
    enum class Difficulty {
        Easy,
        Medium,
        Hard
    };
    
    explicit AIOpponent(QObject *parent = nullptr);
    
    void setGameLogic(GameLogic *logic);
    void setDifficulty(Difficulty difficulty);
    
public slots:
    void makeMove();
    
signals:
    void moveMade(int row, int col);
    
private:
    GameLogic *gameLogic;
    Difficulty difficulty;
    
    void makeRandomMove();
    void makeSmartMove();
    
    int findBestMove();
    int getRandomMove();
    bool isMovesLeft();
    
    // Advanced AI methods (for Hard difficulty)
    int minimax(int depth, bool isMaximizing, int alpha = -1000, int beta = 1000);
    int evaluateBoard();
};

#endif // AIOPPONENT_H 