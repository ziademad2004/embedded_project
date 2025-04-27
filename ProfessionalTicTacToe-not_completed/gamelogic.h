#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include <array>
#include <vector>
#include <QString>
#include <utility>

class GameLogic : public QObject
{
    Q_OBJECT

public:
    enum class Player {
        None,
        X,
        O
    };
    
    enum class GameState {
        InProgress,
        XWins,
        OWins,
        Draw
    };
    
    struct Move {
        int row;
        int col;
        Player player;
    };
    
    typedef std::array<std::array<Player, 3>, 3> Board;
    
    explicit GameLogic(QObject *parent = nullptr);
    ~GameLogic();
    
    void resetGame();
    bool makeMove(int row, int col);
    
    Player getCell(int row, int col) const;
    Player getCurrentPlayer() const;
    GameState getGameState() const;
    const Board& getBoard() const;
    int getMoveCount() const;
    
    QString getBoardString() const;
    bool loadBoard(const QString& boardString);
    
signals:
    void boardUpdated();
    void gameOver(GameState state);
    
private:
    // Game board
    Board board;
    
    // Current player (X or O)
    Player currentPlayer;
    
    // Game state
    GameState gameState;
    
    // Move counter
    int moveCount;
    
    // Check game state
    void checkGameState();
};

#endif // GAMELOGIC_H 