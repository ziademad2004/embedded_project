#ifndef AIOPPONENT_H
#define AIOPPONENT_H

#include <QObject>
#include "gamelogic.h"

class AIOpponent : public QObject {
    Q_OBJECT

public:
    explicit AIOpponent(QObject *parent = nullptr);
    
    void setGameLogic(GameLogic *gameLogic);
    void makeMove();

private:
    GameLogic *m_gameLogic;
};

#endif // AIOPPONENT_H
