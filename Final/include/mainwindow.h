#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QMessageBox>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDialog>

#include "authentication.h"
#include "gamelogic.h"
#include "gamehistory.h"
#include "aiopponent.h"
#include "database.h"

// Custom dialog for game replay
class ReplayDialog : public QDialog {
    Q_OBJECT
public:
    ReplayDialog(QWidget* parent = nullptr);
    void setGameData(const QString& date, const QString& players, 
                    const QString& result, const QString& duration);
    
    // New method to set the move data for replay
    void setMoveData(const QVector<GameMove>& moves);

private slots:
    void onPreviousClicked();
    void onNextClicked();
    void onPlayClicked();
    void onPauseClicked();

private:
    void setupUI();
    void updateMoveCounter();
    void updateBoard();
    void updateButtonStates();
    
    QLabel* m_dateLabel;
    QLabel* m_playersLabel;
    QLabel* m_resultLabel;
    QLabel* m_durationLabel;
    QLabel* m_moveCountLabel;
    
    QVector<QPushButton*> m_cells;
    QPushButton* m_previousBtn;
    QPushButton* m_nextBtn;
    QPushButton* m_playBtn;
    QPushButton* m_pauseBtn;
    QPushButton* m_closeBtn;
    
    int m_currentMove;
    int m_totalMoves;
    QTimer* m_playbackTimer;
    QVector<GameMove> m_moves; // Store the moves for replay
};

// Custom delegate for leaderboard items
class LeaderboardItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    LeaderboardItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// Custom delegate for history items
class HistoryItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    HistoryItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    
signals:
    void replayButtonClicked(int index);
    
private:
    QRect getReplayButtonRect(const QRect& itemRect) const;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCellClicked();
    void onNewGameClicked();
    void onSaveGameClicked();
    void onExitGameClicked();
    void onVsAIClicked();
    void onVsPlayerClicked();
    void onDifficultyButtonClicked();
    void onLoginClicked();
    void onRegisterClicked();
    void onStartPvpGameClicked();
    void onBackToModeClicked();
    void onToggleStatsViewClicked();
    void onBackToGameClicked();
    void onTabClicked(int index);
    void onGameOver(GameLogic::GameResult result);
    void onPlayerChanged(GameLogic::Player player);
    void onBoardChanged();
    void onReplayButtonClicked(int index);

private:
    enum class Screen { ModeSelection, PlayerAuth, Game, Statistics };
    enum class GameMode { None, AI, Player };

    void setupUI();
    void setupConnections();
    void setupGameBoard();
    void setupStyleSheet();
    void setupStatisticsView();
    void setupTitleAnimation(QLabel* titleLabel);
    void showScreen(Screen screen);
    void updateGameStatus(const QString &message);
    void updatePlayerInfo();
    void updateStatistics();
    void populateLeaderboard();
    void highlightWinningCells();
    void addGameToHistory(const QString &result);
    void resetGame();
    void showLoading(int duration = 800);

    QStackedWidget *m_stackedWidget;

    // Screens
    QWidget *m_modeSelectionScreen;
    QWidget *m_playerAuthScreen;
    QWidget *m_gameScreen;
    QWidget *m_statisticsView;

    // Mode Selection Screen
    QPushButton *m_vsAIBtn;
    QPushButton *m_vsPlayerBtn;
    QLineEdit *m_usernameInput;
    QLineEdit *m_passwordInput;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QLabel *m_loginStatus;

    // Player Auth Screen
    QLineEdit *m_player1UsernameInput;
    QLineEdit *m_player1PasswordInput;
    QLineEdit *m_player2UsernameInput;
    QLineEdit *m_player2PasswordInput;
    QPushButton *m_startPvpGameBtn;
    QPushButton *m_backToModeBtn;

    // Game Screen
    QGridLayout *m_boardLayout;
    QVector<QPushButton*> m_cells;
    QLabel *m_statusMessage;
    QPushButton *m_newGameBtn;
    QPushButton *m_saveGameBtn;
    QPushButton *m_exitGameBtn;
    QWidget *m_difficultyContainer;
    QPushButton *m_easyBtn;
    QPushButton *m_mediumBtn;
    QPushButton *m_hardBtn;
    QPushButton *m_expertBtn;
    QListWidget *m_historyList;
    QLabel *m_player1Name;
    QLabel *m_player2Name;
    QWidget *m_player1Box;
    QWidget *m_player2Box;

    // Statistics View
    QLabel *m_statTotalGames;
    QLabel *m_statWins;
    QLabel *m_statLosses;
    QLabel *m_statDraws;
    QLabel *m_statVsAI;
    QLabel *m_statVsPlayers;
    QLabel *m_statWinRate;
    QLabel *m_statBestStreak;
    QListWidget *m_leaderboardList;
    QListWidget *m_fullHistoryList;
    QPushButton *m_toggleStatsViewBtn;
    QPushButton *m_backToGameBtn;
    LeaderboardItemDelegate *m_leaderboardDelegate = nullptr;
    HistoryItemDelegate *m_historyItemDelegate = nullptr;
    
    // Loading Overlay
    QWidget *m_loadingOverlay;
    QLabel *m_loadingSpinner;

    // Game state
    Authentication *m_auth;
    GameLogic *m_gameLogic;
    GameHistory *m_gameHistory;
    AIOpponent *m_aiOpponent;
    Database *m_database;
    GameMode m_gameMode;
    QString m_player1User;
    QString m_player2User;
};

#endif // MAINWINDOW_H
