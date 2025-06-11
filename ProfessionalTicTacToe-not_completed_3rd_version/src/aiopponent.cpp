#include "../include/aiopponent.h"
#include <QTimer>

AIOpponent::AIOpponent(QObject *parent)
    : QObject(parent), m_gameLogic(nullptr)
{
}

void AIOpponent::setGameLogic(GameLogic *gameLogic) {
    m_gameLogic = gameLogic;
}

void AIOpponent::makeMove() {
    if (!m_gameLogic) {
        return;
    }
    
    // Add a slight delay to simulate "thinking" and match the HTML version's behavior
    QTimer::singleShot(700, this, [this]() {
        int moveIndex = m_gameLogic->makeAIMove();
        
        // The move is already made in the GameLogic class, so we don't need to do anything else here
        // This matches the HTML version's behavior where the AI move is handled automatically
        // after the player's move
    });
}
