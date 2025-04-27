#include "../include/aiopponent.h"
#include <QRandomGenerator>
#include <QDebug>

AIOpponent::AIOpponent(QObject *parent)
    : QObject(parent), gameLogic(nullptr), difficulty(Difficulty::Medium)
{
}

void AIOpponent::setGameLogic(GameLogic *logic)
{
    gameLogic = logic;
}

void AIOpponent::setDifficulty(Difficulty difficulty)
{
    this->difficulty = difficulty;
}

void AIOpponent::makeMove()
{
    if (!gameLogic) {
        qWarning() << "Game logic not set in AI opponent";
        return;
    }

    switch (difficulty) {
    case Difficulty::Easy:
        // Easy: Just make a random move
        makeRandomMove();
        break;

    case Difficulty::Medium:
        // Medium: 50% chance of making a smart move, 50% chance of a random move
        if (QRandomGenerator::global()->bounded(2) == 0) {
            makeSmartMove();
        } else {
            makeRandomMove();
        }
        break;

    case Difficulty::Hard:
        // Hard: 80% chance of making a smart move, 20% chance of a random move
        if (QRandomGenerator::global()->bounded(5) < 4) {
            makeSmartMove();
        } else {
            makeRandomMove();
        }
        break;
    }
}

void AIOpponent::makeRandomMove()
{
    int move = getRandomMove();
    if (move != -1) {
        int row = move / 3;
        int col = move % 3;
        emit moveMade(row, col);
    }
}

void AIOpponent::makeSmartMove()
{
    int bestMove = findBestMove();
    if (bestMove != -1) {
        int row = bestMove / 3;
        int col = bestMove % 3;
        emit moveMade(row, col);
    } else {
        // Fallback to random move if no best move found
        makeRandomMove();
    }
}

int AIOpponent::findBestMove()
{
    // First, check if AI can win in one move
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (gameLogic->getCell(i, j) == GameLogic::Player::None) {
                // Save the current state
                auto savedHistory = gameLogic->getMoveHistory();

                // Try this move
                gameLogic->makeMove(i, j);

                // Check if it's a winning move
                if (gameLogic->getGameState() == GameLogic::GameState::OWins) {
                    // Restore the game state
                    gameLogic->resetGame();
                    for (const auto &move : savedHistory) {
                        gameLogic->replayMove(move);
                    }

                    return i * 3 + j;
                }

                // Restore the game state
                gameLogic->resetGame();
                for (const auto &move : savedHistory) {
                    gameLogic->replayMove(move);
                }
            }
        }
    }

    // Next, check if player can win in one move and block it
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (gameLogic->getCell(i, j) == GameLogic::Player::None) {
                // Save the current state
                auto savedHistory = gameLogic->getMoveHistory();

                // Try this move for the player
                gameLogic->makeMove(i, j);

                // Check if it would be a winning move for the player
                if (gameLogic->getGameState() == GameLogic::GameState::XWins) {
                    // Restore the game state
                    gameLogic->resetGame();
                    for (const auto &move : savedHistory) {
                        gameLogic->replayMove(move);
                    }

                    // Block this move
                    return i * 3 + j;
                }

                // Restore the game state
                gameLogic->resetGame();
                for (const auto &move : savedHistory) {
                    gameLogic->replayMove(move);
                }
            }
        }
    }

    // Try to take center if available
    if (gameLogic->getCell(1, 1) == GameLogic::Player::None) {
        return 1 * 3 + 1;
    }

    // Try to take corners
    const int corners[4][2] = {{0, 0}, {0, 2}, {2, 0}, {2, 2}};
    for (const auto &corner : corners) {
        if (gameLogic->getCell(corner[0], corner[1]) == GameLogic::Player::None) {
            return corner[0] * 3 + corner[1];
        }
    }

    // If no strategic move is found, return a random move
    return getRandomMove();
}

int AIOpponent::getRandomMove()
{
    QVector<int> availableMoves;

    // Find all empty cells
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (gameLogic->getCell(i, j) == GameLogic::Player::None) {
                availableMoves.append(i * 3 + j);
            }
        }
    }

    if (availableMoves.isEmpty()) {
        return -1;
    }

    // Return a random move from available moves
    int randomIndex = QRandomGenerator::global()->bounded(availableMoves.size());
    return availableMoves[randomIndex];
}

bool AIOpponent::isMovesLeft()
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (gameLogic->getCell(i, j) == GameLogic::Player::None) {
                return true;
            }
        }
    }
    return false;
}

int AIOpponent::minimax(int depth, bool isMaximizing, int alpha, int beta)
{
    // This is a more advanced minimax algorithm with alpha-beta pruning
    // It's not used in the current implementation but could be implemented
    // for a more challenging AI
    return 0;
}

int AIOpponent::evaluateBoard()
{
    // This method would evaluate the current board state
    // for use in the minimax algorithm
    return 0;
}
