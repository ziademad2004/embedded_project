#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QProgressBar>
#include <QLineEdit>

#include "gamelogic.h"
#include "aiopponent.h"
#include "user.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Game logic slots
    void onCellClicked();
    void updateBoardDisplay();
    void handleGameOver(GameLogic::GameState state);
    void startNewGame();

    // Navigation slots
    void showModeSelectionScreen();
    void showPlayerAuthScreen();
    void showGameScreen();

    // Game mode slots
    void switchToPlayerVsPlayer();
    void switchToPlayerVsAI();
    void startPvpGame();
    void setAIDifficulty();

    // Authentication slots
    void showLoginForm();
    void showRegistrationForm();
    void onLoginSuccess(const User& user);
    void onLogout();

    // Game history slots
    void showGameHistory();
    void replayGame(int gameId);
    void saveGameToHistory();

    // Statistics slots
    void toggleStatisticsView();
    void updateStatistics();
    void populateLeaderboard();

    // UI utility slots
    void showLoadingOverlay(int duration = 1000);
    void updatePlayerInfo();

private:
    // UI setup methods
    void setupUI();
    void setupModeSelectionScreen();
    void setupPlayerAuthScreen();
    void setupGameScreen();
    void setupGameBoard();
    void setupMenuBar();
    void setupStatisticsView();
    void setupLeaderboard();
    void setupConnections();

    // UI components
    QWidget *ui;
    QStackedWidget *mainStack;

    // Game screens
    QWidget *modeSelectionScreen;
    QWidget *playerAuthScreen;
    QWidget *gameScreen;
    QWidget *statisticsView;

    // Game board
    QPushButton *gameBoard[3][3];
    QLabel *statusLabel;

    // User authentication
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QLabel *loginStatusLabel;

    // Game mode selection
    QPushButton *vsAIButton;
    QPushButton *vsPlayerButton;

    // Loading spinner
    QProgressBar *loadingSpinner;

    // Game logic
    GameLogic *gameLogic;
    AIOpponent *aiOpponent;

    // Game state
    bool isLoggedIn;
    bool isPlayingAgainstAI;
    QString currentDifficulty;
    int gameCount;
};

#endif // MAINWINDOW_H
