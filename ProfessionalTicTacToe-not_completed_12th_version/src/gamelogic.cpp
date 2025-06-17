#include "../include/gamelogic.h"
#include <QRandomGenerator>

GameLogic::GameLogic(QObject *parent)
    : QObject(parent), m_currentPlayer(Player::X), m_gameActive(true), m_difficulty(AIDifficulty::Medium)
{
    resetBoard();
}

void GameLogic::resetBoard() {
    m_board.clear();
    m_board.resize(9);
    m_board.fill(Player::None);
    m_currentPlayer = Player::X;
    m_gameActive = true;
    m_winPattern.clear();
    emit boardChanged();
    emit playerChanged(m_currentPlayer);
}

bool GameLogic::makeMove(int index) {
    if (index < 0 || index >= m_board.size() || m_board[index] != Player::None || !m_gameActive) {
        return false;
    }
    
    m_board[index] = m_currentPlayer;
    emit boardChanged();
    
    if (checkWinner()) {
        m_gameActive = false;
        GameResult result = (m_currentPlayer == Player::X) ? GameResult::XWins : GameResult::OWins;
        emit gameOver(result);
        return true;
    }
    
    // Check for draw
    bool boardFull = true;
    const auto& board = m_board;
    for (const Player& cell : board) {
        if (cell == Player::None) {
            boardFull = false;
            break;
        }
    }
    
    if (boardFull) {
        m_gameActive = false;
        emit gameOver(GameResult::Draw);
        return true;
    }
    
    // Switch player
    m_currentPlayer = (m_currentPlayer == Player::X) ? Player::O : Player::X;
    emit playerChanged(m_currentPlayer);
    
    return true;
}

GameLogic::Player GameLogic::getCurrentPlayer() const {
    return m_currentPlayer;
}

GameLogic::Player GameLogic::getCellState(int index) const {
    if (index < 0 || index >= m_board.size()) {
        return Player::None;
    }
    return m_board[index];
}

GameLogic::GameResult GameLogic::getGameResult() const {
    if (!m_gameActive) {
        if (const_cast<GameLogic*>(this)->checkWinner()) {
            return (m_currentPlayer == Player::X) ? GameResult::XWins : GameResult::OWins;
        } else {
            // Check if board is full
            bool boardFull = true;
            const auto& board = m_board;
            for (const Player& cell : board) {
                if (cell == Player::None) {
                    boardFull = false;
                    break;
                }
            }
            
            if (boardFull) {
                return GameResult::Draw;
            }
        }
    }
    
    return GameResult::InProgress;
}

void GameLogic::setAIDifficulty(AIDifficulty difficulty) {
    m_difficulty = difficulty;
}

GameLogic::AIDifficulty GameLogic::getAIDifficulty() const {
    return m_difficulty;
}

QVector<int> GameLogic::getWinPattern() const {
    return m_winPattern;
}

bool GameLogic::checkWinner() {
    const QVector<QVector<int>> winPatterns = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // Rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // Columns
        {0, 4, 8}, {2, 4, 6}              // Diagonals
    };
    
    const auto& patterns = winPatterns;
    for (const QVector<int>& pattern : patterns) {
        const int a = pattern[0];
        const int b = pattern[1];
        const int c = pattern[2];
        
        if (m_board[a] != Player::None && 
            m_board[a] == m_board[b] && 
            m_board[a] == m_board[c]) {
            m_winPattern = pattern;
            return true;
        }
    }
    
    m_winPattern.clear();
    return false;
}

// Test helper methods
void GameLogic::setCellState(int index, Player state) {
    if (index >= 0 && index < m_board.size()) {
        m_board[index] = state;
        emit boardChanged();
    }
}

void GameLogic::setCurrentPlayer(Player player) {
    m_currentPlayer = player;
    emit playerChanged(m_currentPlayer);
}
