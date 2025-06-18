#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include <QVector>

// Structure to represent a game move
struct GameMove {
    int cellIndex;
    int player; // 1 for X, 2 for O
};

class GameLogic : public QObject {
    Q_OBJECT

public:
    enum class Player { None, X, O };
    Q_ENUM(Player)
    
    enum class GameResult { InProgress, XWins, OWins, Draw };
    Q_ENUM(GameResult)
    
    enum class AIDifficulty { Easy, Medium, Hard, Expert };
    Q_ENUM(AIDifficulty)

    explicit GameLogic(QObject *parent = nullptr);

    void resetBoard();
    bool makeMove(int index);
    Player getCurrentPlayer() const;
    Player getCellState(int index) const;
    GameResult getGameResult() const;
    void setAIDifficulty(AIDifficulty difficulty);
    AIDifficulty getAIDifficulty() const;
    QVector<int> getWinPattern() const;
    bool checkWinner();
    
    // New methods for game replay
    QVector<GameMove> getMoveHistory() const;
    void clearMoveHistory();
    
    // Test helper methods
    void setCellState(int index, Player state);
    void setCurrentPlayer(Player player);

signals:
    void boardChanged();
    void gameOver(GameLogic::GameResult result);
    void playerChanged(GameLogic::Player player);

private:
    QVector<Player> m_board;
    Player m_currentPlayer;
    bool m_gameActive;
    AIDifficulty m_difficulty;
    QVector<int> m_winPattern;
    QVector<GameMove> m_moveHistory; // Track moves for replay
};

#endif // GAMELOGIC_H
