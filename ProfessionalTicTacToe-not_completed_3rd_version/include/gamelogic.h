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
    int makeAIMove();
    
    Player getCurrentPlayer() const;
    Player getCellState(int index) const;
    GameResult getGameResult() const;
    void setAIDifficulty(AIDifficulty difficulty);
    AIDifficulty getAIDifficulty() const;
    QVector<int> getWinPattern() const;
    
    bool checkWinner();

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

    int findWinningMove(Player player);
    int makeRandomMove();
    int makeSmartMove();
};

#endif // GAMELOGIC_H
