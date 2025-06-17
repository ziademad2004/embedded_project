#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include <QVector>

class GameLogic : public QObject {
    Q_OBJECT

public:
    enum class Player { None, X, O };
    enum class GameResult { InProgress, XWins, OWins, Draw };
    enum class AIDifficulty { Easy, Medium, Hard, Expert };

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
};

#endif // GAMELOGIC_H
