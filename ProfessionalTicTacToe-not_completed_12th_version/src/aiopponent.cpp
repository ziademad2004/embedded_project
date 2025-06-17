#include "../include/aiopponent.h"
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
using Player = GameLogic::Player;

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
    
    // Add a slight delay to simulate "thinking"
    QTimer::singleShot(700, this, [this]() {
        // Get current board state
        QVector<Player> board;
        for (int i = 0; i < 9; ++i) {
            board.append(m_gameLogic->getCellState(i));
        }
        
        // Debug the board state
        qDebug() << "AI is analyzing board state:";
        for (int i = 0; i < 9; i++) {
            QString cell;
            switch (board[i]) {
                case Player::X: cell = "X"; break;
                case Player::O: cell = "O"; break;
                default: cell = "-"; break;
            }
            qDebug() << "Cell" << i << ":" << cell;
        }
        
        // Determine max depth based on difficulty
        int maxDepth;
        switch (m_gameLogic->getAIDifficulty()) {
            case GameLogic::AIDifficulty::Easy:
                maxDepth = 1;  // Very shallow search for easy
                break;
            case GameLogic::AIDifficulty::Medium:
                maxDepth = 2;  // Medium depth for medium
                break;
            case GameLogic::AIDifficulty::Hard:
                maxDepth = 3;  // Deeper search for hard
                break;
            case GameLogic::AIDifficulty::Expert:
                maxDepth = 9;  // Full search for expert
                break;
            default:
                maxDepth = 2;
        }
        
        // Find best move using minimax
        int bestMove = findBestMove(board, maxDepth);
        qDebug() << "AI chose move:" << bestMove;
        
        // Make the move
        if (bestMove >= 0) {
            // Ensure the game knows it's the AI's turn
            // This is critical because the AI plays as O
            m_gameLogic->makeMove(bestMove);
            
            // Verify the move was made correctly
            Player cellState = m_gameLogic->getCellState(bestMove);
            qDebug() << "After AI move, cell" << bestMove << "state is:" << 
                (cellState == Player::O ? "O" : (cellState == Player::X ? "X" : "-"));
        }
    });
}

// Move ordering: center, corners, edges
static const QVector<int> moveOrder = {4, 0, 2, 6, 8, 1, 3, 5, 7};

bool AIOpponent::isBoardFull(const QVector<Player>& board) {
    for (const Player& cell : board) {
        if (cell == Player::None) {
            return false;
        }
    }
    return true;
}

bool AIOpponent::checkWinner(const QVector<Player>& board, Player player) {
    // Check rows
    for (int i = 0; i < 9; i += 3) {
        if (board[i] == player && board[i+1] == player && board[i+2] == player) {
            return true;
        }
    }
    
    // Check columns
    for (int i = 0; i < 3; ++i) {
        if (board[i] == player && board[i+3] == player && board[i+6] == player) {
            return true;
        }
    }
    
    // Check diagonals
    if (board[0] == player && board[4] == player && board[8] == player) {
        return true;
    }
    if (board[2] == player && board[4] == player && board[6] == player) {
        return true;
    }
    
    return false;
}

int AIOpponent::findBestMove(QVector<Player>& board, int maxDepth) {
    // CRITICAL: Always check for immediate win first (for all difficulty levels)
    for (int i = 0; i < 9; ++i) {
        if (board[i] == Player::None) {
            board[i] = Player::O;
            bool isWin = checkWinner(board, Player::O);
            board[i] = Player::None;
            if (isWin) {
                qDebug() << "AI found winning move at position" << i;
                return i;
            }
        }
    }
    
    // Block opponent's winning move (for medium and above)
    if (m_gameLogic->getAIDifficulty() != GameLogic::AIDifficulty::Easy) {
        for (int i = 0; i < 9; ++i) {
            if (board[i] == Player::None) {
                board[i] = Player::X;
                bool isBlock = checkWinner(board, Player::X);
                board[i] = Player::None;
                if (isBlock) {
                    qDebug() << "AI found blocking move at position" << i;
                    return i;
                }
            }
        }
    }
    
    // For easy, always random; for medium, sometimes random
    if (m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy || 
        (m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium && QRandomGenerator::global()->bounded(100) < 20)) {
        QVector<int> emptyCells;
        for (int i = 0; i < 9; ++i) {
            if (board[i] == Player::None) {
                emptyCells.append(i);
            }
        }
        if (!emptyCells.isEmpty()) {
            int randomMove = emptyCells[QRandomGenerator::global()->bounded(emptyCells.size())];
            qDebug() << "AI chose random move at position" << randomMove;
            return randomMove;
        }
    }
    
    // Minimax with move ordering
    int bestScore = -MAX_SCORE;
    int bestMove = -1;
    for (int i : moveOrder) {
        if (board[i] == Player::None) {
            board[i] = Player::O;
            int score = minimax(board, 0, false, -MAX_SCORE, MAX_SCORE, maxDepth);
            board[i] = Player::None;
            if (score > bestScore) {
                bestScore = score;
                bestMove = i;
            }
        }
    }
    
    qDebug() << "AI chose strategic move at position" << bestMove << "with score" << bestScore;
    return bestMove;
}

int AIOpponent::minimax(QVector<Player>& board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth) {
    // Check for terminal states
    if (checkWinner(board, Player::O)) {
        return WIN_SCORE - depth;  // Prefer winning sooner
    }
    if (checkWinner(board, Player::X)) {
        return LOSE_SCORE + depth;  // Prefer losing later
    }
    if (isBoardFull(board)) {
        return DRAW_SCORE;
    }
    
    // Check depth limit
    if (depth >= maxDepth) {
        return evaluateBoard(board);
    }
    
    if (isMaximizing) {
        int bestScore = -MAX_SCORE;
        for (int i : moveOrder) {
            if (board[i] == Player::None) {
                board[i] = Player::O;
                int score = minimax(board, depth + 1, false, alpha, beta, maxDepth);
                board[i] = Player::None;
                bestScore = qMax(score, bestScore);
                alpha = qMax(alpha, bestScore);
                if (beta <= alpha) {
                    break;  // Beta cutoff
                }
            }
        }
        return bestScore;
    } else {
        int bestScore = MAX_SCORE;
        for (int i : moveOrder) {
            if (board[i] == Player::None) {
                board[i] = Player::X;
                int score = minimax(board, depth + 1, true, alpha, beta, maxDepth);
                board[i] = Player::None;
                bestScore = qMin(score, bestScore);
                beta = qMin(beta, bestScore);
                if (beta <= alpha) {
                    break;  // Alpha cutoff
                }
            }
        }
        return bestScore;
    }
}

int AIOpponent::evaluateBoard(const QVector<Player>& board) {
    // Check rows, columns, and diagonals
    const QVector<QVector<int>> winPatterns = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // Rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // Columns
        {0, 4, 8}, {2, 4, 6}              // Diagonals
    };
    
    int score = 0;
    
    // Evaluate each pattern
    for (const QVector<int>& pattern : winPatterns) {
        int aiCount = 0;
        int playerCount = 0;
        
        for (int pos : pattern) {
            if (board[pos] == Player::O) {
                aiCount++;
            } else if (board[pos] == Player::X) {
                playerCount++;
            }
        }
        
        // Score based on potential winning positions
        if (aiCount == 2 && playerCount == 0) {
            score += 3;  // AI has two in a row
        } else if (playerCount == 2 && aiCount == 0) {
            score -= 3;  // Player has two in a row
        } else if (aiCount == 1 && playerCount == 0) {
            score += 1;  // AI has one in a row
        } else if (playerCount == 1 && aiCount == 0) {
            score -= 1;  // Player has one in a row
        }
    }
    
    // Prefer center position
    if (board[4] == Player::O) {
        score += 2;
    } else if (board[4] == Player::X) {
        score -= 2;
    }
    
    return score;
}
