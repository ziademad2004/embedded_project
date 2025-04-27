#include "../include/gamelogic.h"

GameLogic::GameLogic(QObject *parent)
    : QObject(parent), currentPlayer(Player::X), gameState(GameState::InProgress), moveCount(0)
{
    resetGame();
}

GameLogic::~GameLogic()
{
}

void GameLogic::resetGame()
{
    // Clear the board
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            board[row][col] = Player::None;
        }
    }

    // Reset game state
    currentPlayer = Player::X;
    gameState = GameState::InProgress;
    moveCount = 0;
    moveHistory.clear();

    // Notify that board has been updated
    emit boardUpdated();
}

bool GameLogic::makeMove(int row, int col)
{
    // Check if the move is valid
    if (row < 0 || row >= 3 || col < 0 || col >= 3 ||
        board[row][col] != Player::None ||
        gameState != GameState::InProgress) {
        return false;
    }

    // Make the move
    board[row][col] = currentPlayer;
    moveCount++;

    // Record move in history
    Move move = {row, col, currentPlayer};
    moveHistory.push_back(move);

    // Update game state
    updateGameState();

    // Switch players if game is still in progress
    if (gameState == GameState::InProgress) {
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }

    // Notify that board has been updated
    emit boardUpdated();

    // Notify if game is over
    if (gameState != GameState::InProgress) {
        emit gameOver(gameState);
    }

    return true;
}

bool GameLogic::replayMove(const Move& move)
{
    // Check if the move is valid
    if (move.row < 0 || move.row >= 3 || move.col < 0 || move.col >= 3 ||
        board[move.row][move.col] != Player::None) {
        return false;
    }

    // Make the move
    board[move.row][move.col] = move.player;
    moveCount++;

    // Record move in history
    moveHistory.push_back(move);

    // Update game state
    updateGameState();

    // Switch players
    currentPlayer = (move.player == Player::X) ? Player::O : Player::X;

    // Notify that board has been updated
    emit boardUpdated();

    // Notify if game is over
    if (gameState != GameState::InProgress) {
        emit gameOver(gameState);
    }

    return true;
}

std::vector<GameLogic::Move> GameLogic::getMoveHistory() const
{
    return moveHistory;
}

GameLogic::Player GameLogic::getCell(int row, int col) const
{
    if (row >= 0 && row < 3 && col >= 0 && col < 3) {
        return board[row][col];
    }
    return Player::None;
}

GameLogic::Player GameLogic::getCurrentPlayer() const
{
    return currentPlayer;
}

GameLogic::GameState GameLogic::getGameState() const
{
    return gameState;
}

const GameLogic::Board& GameLogic::getBoard() const
{
    return board;
}

GameLogic::GameState GameLogic::checkGameState() const
{
    return gameState;
}

void GameLogic::updateGameState()
{
    // Check rows
    for (int row = 0; row < 3; row++) {
        if (board[row][0] != Player::None &&
            board[row][0] == board[row][1] &&
            board[row][1] == board[row][2]) {
            gameState = (board[row][0] == Player::X) ? GameState::XWins : GameState::OWins;
            return;
        }
    }

    // Check columns
    for (int col = 0; col < 3; col++) {
        if (board[0][col] != Player::None &&
            board[0][col] == board[1][col] &&
            board[1][col] == board[2][col]) {
            gameState = (board[0][col] == Player::X) ? GameState::XWins : GameState::OWins;
            return;
        }
    }

    // Check diagonals
    if (board[0][0] != Player::None &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2]) {
        gameState = (board[0][0] == Player::X) ? GameState::XWins : GameState::OWins;
        return;
    }

    if (board[0][2] != Player::None &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0]) {
        gameState = (board[0][2] == Player::X) ? GameState::XWins : GameState::OWins;
        return;
    }

    // Check for draw
    if (moveCount >= 9) {
        gameState = GameState::Draw;
        return;
    }

    // Game is still in progress
    gameState = GameState::InProgress;
}

int GameLogic::getMoveCount() const
{
    return moveCount;
}

QString GameLogic::getBoardString() const
{
    QString result;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            if (board[row][col] == Player::X) {
                result += "X";
            } else if (board[row][col] == Player::O) {
                result += "O";
            } else {
                result += "-";
            }
        }
    }

    return result;
}

bool GameLogic::loadBoard(const QString& boardString)
{
    if (boardString.length() != 9) {
        return false;
    }

    int xCount = 0;
    int oCount = 0;

    // Load the board
    for (int i = 0; i < 9; i++) {
        int row = i / 3;
        int col = i % 3;

        QChar c = boardString.at(i);
        if (c == 'X') {
            board[row][col] = Player::X;
            xCount++;
        } else if (c == 'O') {
            board[row][col] = Player::O;
            oCount++;
        } else if (c == '-') {
            board[row][col] = Player::None;
        } else {
            return false;  // Invalid character
        }
    }

    // Determine current player
    if (xCount == oCount) {
        currentPlayer = Player::X;
    } else if (xCount == oCount + 1) {
        currentPlayer = Player::O;
    } else {
        return false;  // Invalid board state
    }

    moveCount = xCount + oCount;
    moveHistory.clear(); // Clear move history as we don't know the sequence

    // Update game state
    updateGameState();

    // Notify that board has been updated
    emit boardUpdated();

    return true;
}
