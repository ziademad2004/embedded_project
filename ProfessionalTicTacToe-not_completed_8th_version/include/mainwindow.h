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

#include "authentication.h"
#include "gamelogic.h"
#include "gamehistory.h"
#include "aiopponent.h"
#include "database.h"

// Custom delegate for leaderboard items
class LeaderboardItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    LeaderboardItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// Custom delegate for history items
class HistoryItemDelegate : public QStyledItemDelegate {
public:
    HistoryItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
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
