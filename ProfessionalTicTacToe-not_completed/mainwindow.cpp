#include "../include/mainwindow.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateTime>
#include <QTabWidget>
#include <QFile>
#include <QTimer>

#include "../include/authentication.h"
#include "../include/gamehistory.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(nullptr), gameLogic(new GameLogic(this)), 
      aiOpponent(new AIOpponent(this)), isLoggedIn(false), isPlayingAgainstAI(false),
      currentDifficulty("medium"), gameCount(0)
{
    // Set window title and size
    setWindowTitle("Professional Tic Tac Toe");
    resize(900, 700);
    
    // Load stylesheet
    QFile styleFile(":/resources/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        this->setStyleSheet(style);
        styleFile.close();
    }
    
    // Setup the complete UI
    setupUI();
    
    // Initialize game
    gameLogic->resetGame();
    
    // Set AI opponent's game logic
    aiOpponent->setGameLogic(gameLogic);
    aiOpponent->setDifficulty(AIOpponent::Difficulty::Medium);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // Create main widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create stacked widget for main screens
    mainStack = new QStackedWidget(centralWidget);
    mainLayout->addWidget(mainStack);
    
    // Setup all screens
    setupModeSelectionScreen();
    setupPlayerAuthScreen();
    setupGameScreen();
    setupStatisticsView();
    
    // Setup game board
    setupGameBoard();
    
    // Setup menu bar
    setupMenuBar();
    
    // Setup connections
    setupConnections();
    
    // Create loading spinner (initially hidden)
    loadingSpinner = new QProgressBar(this);
    loadingSpinner->setObjectName("spinner");
    loadingSpinner->setRange(0, 0); // Indeterminate spinner
    loadingSpinner->setTextVisible(false);
    loadingSpinner->setFixedSize(100, 100);
    loadingSpinner->setAlignment(Qt::AlignCenter);
    loadingSpinner->hide();
    
    // Show mode selection screen initially
    showModeSelectionScreen();
}

void MainWindow::setupModeSelectionScreen()
{
    modeSelectionScreen = new QWidget(mainStack);
    QVBoxLayout *layout = new QVBoxLayout(modeSelectionScreen);
    
    // Title
    QLabel *titleLabel = new QLabel("PROFESSIONAL TIC TAC TOE", modeSelectionScreen);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Mode selection
    QFrame *modeSelectionFrame = new QFrame(modeSelectionScreen);
    modeSelectionFrame->setProperty("class", "panel");
    QVBoxLayout *modeLayout = new QVBoxLayout(modeSelectionFrame);
    
    QLabel *modeTitle = new QLabel("Select Game Mode", modeSelectionFrame);
    modeTitle->setProperty("class", "panel-title");
    modeLayout->addWidget(modeTitle);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    vsAIButton = new QPushButton("Player vs AI", modeSelectionFrame);
    vsPlayerButton = new QPushButton("Player vs Player", modeSelectionFrame);
    vsAIButton->setProperty("class", "modeButton");
    vsPlayerButton->setProperty("class", "modeButton");
    
    buttonLayout->addWidget(vsAIButton);
    buttonLayout->addWidget(vsPlayerButton);
    modeLayout->addLayout(buttonLayout);
    
    layout->addWidget(modeSelectionFrame);
    
    // Authentication panel
    QFrame *authFrame = new QFrame(modeSelectionScreen);
    authFrame->setProperty("class", "panel");
    QVBoxLayout *authLayout = new QVBoxLayout(authFrame);
    
    QLabel *authTitle = new QLabel("User Authentication", authFrame);
    authTitle->setProperty("class", "panel-title");
    authLayout->addWidget(authTitle);
    
    usernameEdit = new QLineEdit(authFrame);
    usernameEdit->setPlaceholderText("Username");
    authLayout->addWidget(usernameEdit);
    
    passwordEdit = new QLineEdit(authFrame);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    authLayout->addWidget(passwordEdit);
    
    QHBoxLayout *authButtonLayout = new QHBoxLayout();
    loginButton = new QPushButton("Login", authFrame);
    registerButton = new QPushButton("Register", authFrame);
    
    authButtonLayout->addWidget(loginButton);
    authButtonLayout->addWidget(registerButton);
    authLayout->addLayout(authButtonLayout);
    
    loginStatusLabel = new QLabel("", authFrame);
    loginStatusLabel->setObjectName("loginStatus");
    authLayout->addWidget(loginStatusLabel);
    
    layout->addWidget(authFrame);
    
    // Add to stack
    mainStack->addWidget(modeSelectionScreen);
}

void MainWindow::setupPlayerAuthScreen()
{
    playerAuthScreen = new QWidget(mainStack);
    // Implementation omitted for brevity
    mainStack->addWidget(playerAuthScreen);
}

void MainWindow::setupGameScreen()
{
    gameScreen = new QWidget(mainStack);
    // Implementation omitted for brevity
    mainStack->addWidget(gameScreen);
}

void MainWindow::setupGameBoard()
{
    // Find the game board frame in the game screen
    QFrame *gameBoardFrame = gameScreen->findChild<QFrame*>();
    
    if (!gameBoardFrame) {
        // No game board frame found yet, create minimal layout for compilation
        QVBoxLayout *layout = new QVBoxLayout(gameScreen);
        gameBoardFrame = new QFrame(gameScreen);
        layout->addWidget(gameBoardFrame);
        
        QVBoxLayout *gameBoardLayout = new QVBoxLayout(gameBoardFrame);
        
        // Add status label
        statusLabel = new QLabel("Your turn (X)", gameBoardFrame);
        statusLabel->setObjectName("statusLabel");
        gameBoardLayout->addWidget(statusLabel);
        
        // Add grid for game board
        QGridLayout *gridLayout = new QGridLayout();
        gameBoardLayout->addLayout(gridLayout);
        
        // Create buttons for each cell
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                gameBoard[row][col] = new QPushButton("", gameBoardFrame);
                gameBoard[row][col]->setFixedSize(100, 100);
                
                QFont font = gameBoard[row][col]->font();
                font.setPointSize(24);
                font.setBold(true);
                gameBoard[row][col]->setFont(font);
                
                gameBoard[row][col]->setProperty("row", row);
                gameBoard[row][col]->setProperty("col", col);
                
                connect(gameBoard[row][col], &QPushButton::clicked, this, &MainWindow::onCellClicked);
                
                gridLayout->addWidget(gameBoard[row][col], row, col);
            }
        }
    }
}

void MainWindow::setupMenuBar()
{
    // Create menu bar
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // Create game menu
    QMenu *gameMenu = menuBar->addMenu("Game");
    QAction *newGameAction = gameMenu->addAction("New Game");
    gameMenu->addSeparator();
    QAction *exitAction = gameMenu->addAction("Exit");
    
    connect(newGameAction, &QAction::triggered, this, &MainWindow::startNewGame);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupStatisticsView()
{
    statisticsView = new QWidget();
    // Implementation omitted for brevity
}

void MainWindow::setupConnections()
{
    // Connect game logic signals
    connect(gameLogic, &GameLogic::boardUpdated, this, &MainWindow::updateBoardDisplay);
    connect(gameLogic, &GameLogic::gameOver, this, &MainWindow::handleGameOver);
    
    // More connections would be implemented here
}

// Slot implementations
void MainWindow::onCellClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        int row = button->property("row").toInt();
        int col = button->property("col").toInt();
        
        gameLogic->makeMove(row, col);
    }
}

void MainWindow::updateBoardDisplay()
{
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            GameLogic::Player cellState = gameLogic->getCell(row, col);
            QString text = "";
            
            if (cellState == GameLogic::Player::X) {
                text = "X";
                gameBoard[row][col]->setProperty("player", "x");
            } else if (cellState == GameLogic::Player::O) {
                text = "O";
                gameBoard[row][col]->setProperty("player", "o");
            } else {
                gameBoard[row][col]->setProperty("player", "");
            }
            
            gameBoard[row][col]->setText(text);
        }
    }
}

void MainWindow::handleGameOver(GameLogic::GameState state)
{
    QString message;
    
    switch (state) {
        case GameLogic::GameState::XWins:
            message = "X wins!";
            break;
        case GameLogic::GameState::OWins:
            message = "O wins!";
            break;
        case GameLogic::GameState::Draw:
            message = "It's a draw!";
            break;
        default:
            return;
    }
    
    QMessageBox::information(this, "Game Over", message);
}

void MainWindow::startNewGame()
{
    gameLogic->resetGame();
    updateBoardDisplay();
}

void MainWindow::showModeSelectionScreen()
{
    mainStack->setCurrentWidget(modeSelectionScreen);
}

void MainWindow::showPlayerAuthScreen()
{
    mainStack->setCurrentWidget(playerAuthScreen);
}

void MainWindow::showGameScreen()
{
    mainStack->setCurrentWidget(gameScreen);
}

// Stub implementations for required slots
void MainWindow::switchToPlayerVsPlayer() {}
void MainWindow::switchToPlayerVsAI() {}
void MainWindow::showLoginForm() {}
void MainWindow::showRegistrationForm() {}
void MainWindow::onLoginSuccess(const User& user) {}
void MainWindow::onLogout() {}
void MainWindow::showGameHistory() {}
void MainWindow::replayGame(int gameId) {}
void MainWindow::saveGameToHistory() {}
void MainWindow::toggleStatisticsView() {}
void MainWindow::setAIDifficulty() {}
void MainWindow::startPvpGame() {}
void MainWindow::updateStatistics() {}
void MainWindow::populateLeaderboard() {}
void MainWindow::showLoadingOverlay(int duration) {}
void MainWindow::updatePlayerInfo() {}
void MainWindow::setupLeaderboard() {} 