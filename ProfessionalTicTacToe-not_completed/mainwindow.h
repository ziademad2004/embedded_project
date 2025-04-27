#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QProgressBar>

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
    void onCellClicked();
    void updateBoardDisplay();
    void handleGameOver(GameLogic::GameState state);
    void startNewGame();
    
    void showModeSelectionScreen();
    void showPlayerAuthScreen();
    void showGameScreen();
    
    void switchToPlayerVsPlayer();
    void switchToPlayerVsAI();
    void showLoginForm();
    void showRegistrationForm();
    void onLoginSuccess(const User& user);
    void onLogout();
    
    void showGameHistory();
    void replayGame(int gameId);
    void saveGameToHistory();
    
    void toggleStatisticsView();
    void setAIDifficulty();
    void startPvpGame();
    void updateStatistics();
    void populateLeaderboard();
    
    void showLoadingOverlay(int duration = 1000);
    void updatePlayerInfo();

private:
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