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
    QFile styleFile(":/styles/main.qss");
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
    // ui pointer is deleted automatically if it's a parent-child relationship
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
    modeSelectionScreen = new QWidget();
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
    playerAuthScreen = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(playerAuthScreen);

    QLabel *title = new QLabel("Player Authentication", playerAuthScreen);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Add a simple message
    QLabel *message = new QLabel("Please log in or register to play", playerAuthScreen);
    message->setAlignment(Qt::AlignCenter);
    layout->addWidget(message);

    // Add to stack
    mainStack->addWidget(playerAuthScreen);
}

void MainWindow::setupGameScreen()
{
    gameScreen = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(gameScreen);

    QLabel *title = new QLabel("Game Screen", gameScreen);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Create a frame for the game board
    QFrame *gameBoardFrame = new QFrame(gameScreen);
    gameBoardFrame->setFrameShape(QFrame::StyledPanel);
    gameBoardFrame->setFrameShadow(QFrame::Raised);
    layout->addWidget(gameBoardFrame);

    // Layout for the game board frame will be set up in setupGameBoard

    // Add to stack
    mainStack->addWidget(gameScreen);
}

void MainWindow::setupGameBoard()
{
    // Find the game board frame in the game screen
    QFrame *gameBoardFrame = nullptr;
    for (QObject *child : gameScreen->children()) {
        QFrame *frame = qobject_cast<QFrame*>(child);
        if (frame && frame->frameShape() == QFrame::StyledPanel) {
            gameBoardFrame = frame;
            break;
        }
    }

    if (gameBoardFrame) {
        QVBoxLayout *gameBoardLayout = new QVBoxLayout(gameBoardFrame);

        // Add status label
        statusLabel = new QLabel("Your turn (X)", gameBoardFrame);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setAlignment(Qt::AlignCenter);
        gameBoardLayout->addWidget(statusLabel);

        // Add grid for game board
        QGridLayout *gridLayout = new QGridLayout();
        gridLayout->setSpacing(10);
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
    QVBoxLayout *layout = new QVBoxLayout(statisticsView);

    QLabel *title = new QLabel("Game Statistics", statisticsView);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Create a tab widget for different statistics views
    QTabWidget *tabWidget = new QTabWidget(statisticsView);
    layout->addWidget(tabWidget);

    // Personal stats tab
    QWidget *personalStatsTab = new QWidget();
    tabWidget->addTab(personalStatsTab, "My Stats");

    // Leaderboard tab
    QWidget *leaderboardTab = new QWidget();
    tabWidget->addTab(leaderboardTab, "Leaderboard");

    // Game history tab
    QWidget *historyTab = new QWidget();
    tabWidget->addTab(historyTab, "Game History");

    // Setup the leaderboard content
    setupLeaderboard();

    // Add to stack
    mainStack->addWidget(statisticsView);
}

void MainWindow::setupLeaderboard()
{
    // This will be implemented later for the leaderboard feature
}

void MainWindow::setupConnections()
{
    // Connect game logic signals
    connect(gameLogic, &GameLogic::boardUpdated, this, &MainWindow::updateBoardDisplay);
    connect(gameLogic, &GameLogic::gameOver, this, &MainWindow::handleGameOver);

    // Connect mode selection buttons
    connect(vsPlayerButton, &QPushButton::clicked, this, &MainWindow::switchToPlayerVsPlayer);
    connect(vsAIButton, &QPushButton::clicked, this, &MainWindow::switchToPlayerVsAI);

    // Setup initial authentication form (login by default)
    showLoginForm();

    // Connect authentication signals
    connect(&Authentication::getInstance(), &Authentication::loginSuccessful,
            this, &MainWindow::onLoginSuccess);
    connect(&Authentication::getInstance(), &Authentication::loginFailed,
            this, [this](const QString &reason) {
                loginStatusLabel->setText("Login failed: " + reason);
            });
    connect(&Authentication::getInstance(), &Authentication::registrationSuccessful,
            this, [this]() {
                loginStatusLabel->setText("Registration successful! Please log in.");
                QTimer::singleShot(1500, this, &MainWindow::showLoginForm);
            });
    connect(&Authentication::getInstance(), &Authentication::registrationFailed,
            this, [this](const QString &reason) {
                loginStatusLabel->setText("Registration failed: " + reason);
            });

    // Connect AI opponent signals
    connect(aiOpponent, &AIOpponent::moveMade, this, [this](int row, int col) {
        gameLogic->makeMove(row, col);
    });
}

// Slot implementations
void MainWindow::onCellClicked()
{
    // Get the button that was clicked using sender()
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        // Extract row and col from button properties
        int row = button->property("row").toInt();
        int col = button->property("col").toInt();
        // Make the move in the game logic
        gameLogic->makeMove(row, col);
        // If playing against AI and move was successful, let AI make a move
        if (isPlayingAgainstAI && gameLogic->getGameState() == GameLogic::GameState::InProgress) {
            // Small delay before AI makes a move
            QTimer::singleShot(500, aiOpponent, &AIOpponent::makeMove);
        }
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
            QStyle *buttonStyle = gameBoard[row][col]->style();
            if (buttonStyle) {
                buttonStyle->unpolish(gameBoard[row][col]);
                buttonStyle->polish(gameBoard[row][col]);
            }
        }
    }

    // Update status label
    if (gameLogic->getGameState() == GameLogic::GameState::InProgress) {
        if (gameLogic->getCurrentPlayer() == GameLogic::Player::X) {
            statusLabel->setText("X's turn");
        } else {
            statusLabel->setText("O's turn");
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

    // Update status label
    statusLabel->setText(message);

    // Show message box
    QMessageBox::information(this, "Game Over", message);

    // Save game to history if logged in
    if (isLoggedIn) {
        saveGameToHistory();
    }
}

void MainWindow::startNewGame()
{
    // Reset game logic
    gameLogic->resetGame();

    // Update display
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

void MainWindow::switchToPlayerVsPlayer()
{
    isPlayingAgainstAI = false;

    // If user is logged in, go straight to game
    if (isLoggedIn) {
        startPvpGame();
    } else {
        // Show authentication screen
        showPlayerAuthScreen();
    }
}

void MainWindow::switchToPlayerVsAI()
{
    isPlayingAgainstAI = true;

    // If user is logged in, go straight to game
    if (isLoggedIn) {
        startNewGame();
        showGameScreen();
    } else {
        // Show authentication screen
        showPlayerAuthScreen();
    }
}

void MainWindow::showLoginForm()
{
    // Clear fields
    usernameEdit->clear();
    passwordEdit->clear();
    loginStatusLabel->clear();

    // Change status to show we're in login mode
    loginStatusLabel->setText("Please enter your credentials");

    // Connect buttons to login function
    disconnect(loginButton, &QPushButton::clicked, nullptr, nullptr);
    disconnect(registerButton, &QPushButton::clicked, nullptr, nullptr);

    connect(loginButton, &QPushButton::clicked, this, [this]() {
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();

        if (username.isEmpty() || password.isEmpty()) {
            loginStatusLabel->setText("Username and password cannot be empty");
            return;
        }

        // Show loading
        showLoadingOverlay();

        // Try login
        bool success = Authentication::getInstance().loginUser(username, password);

        if (!success) {
            loginStatusLabel->setText("Login failed. Check your credentials.");
        }
    });

    // Make register button switch to registration form
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::showRegistrationForm);
}

void MainWindow::showRegistrationForm()
{
    // Clear fields
    usernameEdit->clear();
    passwordEdit->clear();
    loginStatusLabel->clear();

    // Change status to show we're in registration mode
    loginStatusLabel->setText("Create a new account");

    // Connect buttons to registration function
    disconnect(loginButton, &QPushButton::clicked, nullptr, nullptr);
    disconnect(registerButton, &QPushButton::clicked, nullptr, nullptr);

    // Make login button switch back to login form
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::showLoginForm);

    // Make register button register a new user
    connect(registerButton, &QPushButton::clicked, this, [this]() {
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();

        if (username.isEmpty() || password.isEmpty()) {
            loginStatusLabel->setText("Username and password cannot be empty");
            return;
        }

        // Show loading
        showLoadingOverlay();

        // Try registration
        bool success = Authentication::getInstance().registerUser(username, password, "");

        if (success) {
            loginStatusLabel->setText("Registration successful! Please log in.");
            QTimer::singleShot(1500, this, &MainWindow::showLoginForm);
        } else {
            loginStatusLabel->setText("Registration failed. Try a different username.");
        }
    });
}

void MainWindow::onLoginSuccess(const User& user)
{
    // Update login state
    isLoggedIn = true;

    // Update UI to show logged in user
    loginStatusLabel->setText("Logged in as " + user.getUsername());

    // Show appropriate screen based on selected game mode
    if (isPlayingAgainstAI) {
        startNewGame();
        showGameScreen();
    } else {
        startPvpGame();
    }

    // Update player info in UI
    updatePlayerInfo();
}

void MainWindow::onLogout()
{
    // Update login state
    isLoggedIn = false;

    // Reset UI
    loginStatusLabel->setText("");

    // Show mode selection screen
    showModeSelectionScreen();
}

void MainWindow::showGameHistory()
{
    if (!isLoggedIn) {
        QMessageBox::information(this, "Login Required", "Please log in to view game history.");
        return;
    }

    // Implementation would display game history for the logged-in user
}

void MainWindow::replayGame(int gameId)
{
    // This would load and replay a saved game
}

void MainWindow::saveGameToHistory()
{
    // This would save the current game to the history database
    if (isLoggedIn) {
        User currentUser = Authentication::getInstance().getCurrentUser();

        // Save to game history
        GameHistory::getInstance().saveGame(
            currentUser,
            isPlayingAgainstAI,
            isPlayingAgainstAI ? "AI" : "Player 2",
            gameLogic->getGameState(),
            gameLogic->getMoveHistory()
            );
    }
}

void MainWindow::toggleStatisticsView()
{
    if (mainStack->currentWidget() == statisticsView) {
        showGameScreen();
    } else {
        mainStack->setCurrentWidget(statisticsView);
    }
}

void MainWindow::setAIDifficulty()
{
    // This would set the AI difficulty level
}

void MainWindow::startPvpGame()
{
    // Reset game logic
    gameLogic->resetGame();

    // Show game screen
    showGameScreen();

    // Update status label
    statusLabel->setText("Player 1's turn (X)");
}

void MainWindow::updateStatistics()
{
    // This would update the statistics display
}

void MainWindow::populateLeaderboard()
{
    // This would populate the leaderboard with top players
}

void MainWindow::showLoadingOverlay(int duration)
{
    // Position spinner in the center
    QPoint center = this->rect().center();
    loadingSpinner->move(center.x() - loadingSpinner->width()/2, center.y() - loadingSpinner->height()/2);

    // Show spinner
    loadingSpinner->show();

    // Set timer to hide spinner after duration
    QTimer::singleShot(duration, this, [this]() {
        loadingSpinner->hide();
    });
}

void MainWindow::updatePlayerInfo()
{
    if (isLoggedIn) {
        User user = Authentication::getInstance().getCurrentUser();

        // Format user info with stats
        QString info = QString("Player: %1 | Wins: %2 | Losses: %3 | Draws: %4")
                           .arg(user.getUsername())
                           .arg(user.getWins())
                           .arg(user.getLosses())
                           .arg(user.getDraws());

        // Update UI with user info (e.g., a status bar or label)
        // This depends on your UI design
        // For now, we'll just update the window title
        setWindowTitle("Professional Tic Tac Toe - " + user.getUsername());
    } else {
        setWindowTitle("Professional Tic Tac Toe");
    }
}
