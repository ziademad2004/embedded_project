#include "../include/mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QMovie>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QStackedWidget>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QPen>
#include <QFont>
#include <QListWidgetItem>

// Implementation of LeaderboardItemDelegate::paint method
void LeaderboardItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }
    
    // Draw the background
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(0, 238, 255, 50));
    } else {
        painter->fillRect(option.rect, QColor(0, 136, 255, 25));
    }
    
    // Draw the border
    painter->setPen(QPen(QColor("#0088ff"), 1));
    painter->drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), 5, 5);
    
    // Get the text data
    QString playerRank = index.data(Qt::DisplayRole).toString();
    QString stats = index.data(Qt::UserRole).toString();
    
    // Set up the font
    QFont playerFont = option.font;
    playerFont.setBold(true);
    QFont statsFont = option.font;
    
    // Calculate text rectangles
    QRect rankRect = option.rect.adjusted(10, 0, -10, 0);
    QRect statsRect = option.rect.adjusted(10, 0, -10, 0);
    
    // Draw the player rank with cyan color
    painter->setPen(QColor("#00eeff"));
    painter->setFont(playerFont);
    painter->drawText(rankRect, Qt::AlignLeft | Qt::AlignVCenter, playerRank);
    
    // Draw the stats with white color
    painter->setPen(Qt::white);
    painter->setFont(statsFont);
    painter->drawText(statsRect, Qt::AlignRight | Qt::AlignVCenter, stats);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Initialize game components
    m_auth = new Authentication(this);
    m_gameLogic = new GameLogic(this);
    m_gameHistory = new GameHistory(this);
    m_aiOpponent = new AIOpponent(this);
    m_database = new Database(this);

    m_aiOpponent->setGameLogic(m_gameLogic);
    m_gameMode = GameMode::None;

    // We'll make everything compact through layout adjustments

    setupUI();
    setupConnections();
    setupStyleSheet();

    // Start with mode selection screen
    showScreen(Screen::ModeSelection);
}

MainWindow::~MainWindow() {
    // No need to delete ui since we're not using it
}

void MainWindow::setupUI() {
    // Create main stacked widget
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // Create screens
    m_modeSelectionScreen = new QWidget();
    m_playerAuthScreen = new QWidget();
    m_gameScreen = new QWidget();
    m_statisticsView = new QWidget();

    m_stackedWidget->addWidget(m_modeSelectionScreen);
    m_stackedWidget->addWidget(m_playerAuthScreen);
    m_stackedWidget->addWidget(m_gameScreen);

    // Create mode selection screen
    QVBoxLayout *modeLayout = new QVBoxLayout(m_modeSelectionScreen);
    // Set smaller margins for a more compact UI
    modeLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel("Professional Tic Tac Toe");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    // Apply extra intense neon styling with white core
    titleLabel->setStyleSheet("color: #00ffff; text-shadow: 0 0 2px #fff, 0 0 4px #fff, 0 0 6px #fff, 0 0 8px #fff, 0 0 10px #00ffff, 0 0 15px #00ffff, 0 0 20px #00ffff, 0 0 25px #00ffff, 0 0 30px #00ffff, 0 0 35px #00ffff, 0 0 40px #00ffff, 0 0 50px #00ffff, 0 0 60px #00ffff, 0 0 70px #00ffff; font-weight: 800; letter-spacing: 3px; font-size: 42px;");
    // Setup pulsating neon animation
    setupTitleAnimation(titleLabel);
    modeLayout->addWidget(titleLabel);

    QWidget *modeSelectionPanel = new QWidget();
    modeSelectionPanel->setObjectName("modeSelection");
    QVBoxLayout *modeSelectionLayout = new QVBoxLayout(modeSelectionPanel);
    modeSelectionLayout->setSpacing(12); // Reduced spacing

    QLabel *modeSelectionTitle = new QLabel("Select Game Mode");
    modeSelectionTitle->setObjectName("panelTitle");
    modeSelectionTitle->setAlignment(Qt::AlignCenter); // Center align title
    modeSelectionLayout->addWidget(modeSelectionTitle);
    modeSelectionLayout->addSpacing(5); // Reduced space after title

    QHBoxLayout *modeButtonsLayout = new QHBoxLayout();
    modeButtonsLayout->setSpacing(10); // Reduced spacing between buttons
    m_vsAIBtn = new QPushButton("Player vs AI");
    m_vsAIBtn->setObjectName("modeButton");
    m_vsPlayerBtn = new QPushButton("Player vs Player");
    m_vsPlayerBtn->setObjectName("modeButton");

    modeButtonsLayout->addWidget(m_vsAIBtn);
    modeButtonsLayout->addWidget(m_vsPlayerBtn);
    modeSelectionLayout->addLayout(modeButtonsLayout);

    modeLayout->addWidget(modeSelectionPanel);
    
    // Add spacing between the mode selection panel and auth panel
    modeLayout->addSpacing(40);

    // Create auth panel
    QWidget *authPanel = new QWidget();
    authPanel->setObjectName("authPanel");
    QVBoxLayout *authLayout = new QVBoxLayout(authPanel);
    authLayout->setSpacing(10); // Reduced spacing

    QLabel *authTitle = new QLabel("User Authentication");
    authTitle->setObjectName("panelTitle");
    authTitle->setAlignment(Qt::AlignCenter); // Center align title
    authLayout->addWidget(authTitle);
    authLayout->addSpacing(5); // Reduced space after title

    m_usernameInput = new QLineEdit();
    m_usernameInput->setPlaceholderText("Username");
    m_usernameInput->setMinimumHeight(32); // Reduced height for input fields
    m_passwordInput = new QLineEdit();
    m_passwordInput->setPlaceholderText("Password");
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setMinimumHeight(32); // Reduced height for input fields

    authLayout->addWidget(m_usernameInput);
    authLayout->addWidget(m_passwordInput);
    authLayout->addSpacing(5); // Add spacing before buttons

    QHBoxLayout *authButtonsLayout = new QHBoxLayout();
    authButtonsLayout->setSpacing(15); // Add spacing between buttons
    m_loginBtn = new QPushButton("Login");
    m_registerBtn = new QPushButton("Register");

    authButtonsLayout->addWidget(m_loginBtn);
    authButtonsLayout->addStretch(); // Add stretch to push buttons to the sides
    authButtonsLayout->addWidget(m_registerBtn);
    authLayout->addLayout(authButtonsLayout);

    m_loginStatus = new QLabel("");
    m_loginStatus->setObjectName("loginStatus");
    m_loginStatus->setProperty("status", "success"); // Default status
    authLayout->addWidget(m_loginStatus);

    modeLayout->addWidget(authPanel);

    // Setup player auth screen
    QVBoxLayout *playerAuthLayout = new QVBoxLayout(m_playerAuthScreen);

    QWidget *playerAuthPanel = new QWidget();
    playerAuthPanel->setObjectName("playerAuthPanel");
    QVBoxLayout *playerAuthPanelLayout = new QVBoxLayout(playerAuthPanel);

    QLabel *playerAuthTitle = new QLabel("Player Authentication");
    playerAuthTitle->setObjectName("panelTitle");
    playerAuthPanelLayout->addWidget(playerAuthTitle);

    // Player 1 inputs
    QLabel *player1Label = new QLabel("Player 1 (X)");
    m_player1UsernameInput = new QLineEdit();
    m_player1UsernameInput->setPlaceholderText("Username");
    m_player1PasswordInput = new QLineEdit();
    m_player1PasswordInput->setPlaceholderText("Password");
    m_player1PasswordInput->setEchoMode(QLineEdit::Password);

    playerAuthPanelLayout->addWidget(player1Label);
    playerAuthPanelLayout->addWidget(m_player1UsernameInput);
    playerAuthPanelLayout->addWidget(m_player1PasswordInput);

    // Player 2 inputs
    QLabel *player2Label = new QLabel("Player 2 (O)");
    m_player2UsernameInput = new QLineEdit();
    m_player2UsernameInput->setPlaceholderText("Username");
    m_player2PasswordInput = new QLineEdit();
    m_player2PasswordInput->setPlaceholderText("Password");
    m_player2PasswordInput->setEchoMode(QLineEdit::Password);

    playerAuthPanelLayout->addWidget(player2Label);
    playerAuthPanelLayout->addWidget(m_player2UsernameInput);
    playerAuthPanelLayout->addWidget(m_player2PasswordInput);

    // Auth buttons
    QHBoxLayout *pvpAuthButtonsLayout = new QHBoxLayout();
    m_startPvpGameBtn = new QPushButton("Start Game");
    m_backToModeBtn = new QPushButton("Back");

    pvpAuthButtonsLayout->addWidget(m_startPvpGameBtn);
    pvpAuthButtonsLayout->addWidget(m_backToModeBtn);
    playerAuthPanelLayout->addLayout(pvpAuthButtonsLayout);

    playerAuthLayout->addWidget(playerAuthPanel);

    // Create game screen
    setupGameBoard();

    // Create loading overlay
    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setObjectName("loadingOverlay");
    m_loadingOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.8);");
    m_loadingOverlay->setFixedSize(this->size());
    m_loadingOverlay->setVisible(false);

    QVBoxLayout *loadingLayout = new QVBoxLayout(m_loadingOverlay);
    loadingLayout->setAlignment(Qt::AlignCenter);

    m_loadingSpinner = new QLabel(m_loadingOverlay);
    m_loadingSpinner->setObjectName("loadingSpinner");
    m_loadingSpinner->setMinimumSize(50, 50);
    m_loadingSpinner->setMaximumSize(50, 50);
    m_loadingSpinner->setStyleSheet("border: 5px solid rgba(0, 255, 255, 0.3); border-radius: 25px; border-top-color: #00ffff;");

    // Add spinner animation
    QMovie *spinnerMovie = new QMovie(":/images/spinner.gif");
    m_loadingSpinner->setMovie(spinnerMovie);
    spinnerMovie->start();

    loadingLayout->addWidget(m_loadingSpinner);
}

void MainWindow::setupGameBoard() {
    QHBoxLayout *gameLayout = new QHBoxLayout(m_gameScreen);

    // Title - Professional Tic Tac Toe
    QLabel *gameTitleLabel = new QLabel("Professional Tic Tac Toe");
    gameTitleLabel->setObjectName("titleLabel");
    gameTitleLabel->setAlignment(Qt::AlignCenter);
    // Apply extra intense neon styling with white core
    gameTitleLabel->setStyleSheet("color: #00ffff; text-shadow: 0 0 2px #fff, 0 0 4px #fff, 0 0 6px #fff, 0 0 8px #fff, 0 0 10px #00ffff, 0 0 15px #00ffff, 0 0 20px #00ffff, 0 0 25px #00ffff, 0 0 30px #00ffff, 0 0 35px #00ffff, 0 0 40px #00ffff, 0 0 50px #00ffff, 0 0 60px #00ffff, 0 0 70px #00ffff; font-weight: 800; letter-spacing: 3px; font-size: 42px;");
    // Setup pulsating neon animation
    setupTitleAnimation(gameTitleLabel);

    // Create the main container
    QWidget *gameContainer = new QWidget();
    QHBoxLayout *gameContainerLayout = new QHBoxLayout(gameContainer);
    gameContainerLayout->setSpacing(15); // Add spacing between left and right sides

    // Left side container - player info, difficulty, history
    QWidget *leftSideContainer = new QWidget();
    QVBoxLayout *leftSideLayout = new QVBoxLayout(leftSideContainer);
    leftSideLayout->setSpacing(15); // Add spacing between sections

    // Player info boxes side by side
    QHBoxLayout *playerInfoLayout = new QHBoxLayout();
    playerInfoLayout->setSpacing(15); // Space between player boxes

    // Player 1 Box
    m_player1Box = new QWidget();
    m_player1Box->setObjectName("playerBox");
    m_player1Box->setProperty("current-player", true);
    m_player1Box->setFixedWidth(180);
    QVBoxLayout *player1BoxLayout = new QVBoxLayout(m_player1Box);
    QLabel *player1Label = new QLabel("Player 1 (X)");
    player1Label->setAlignment(Qt::AlignCenter);
    m_player1Name = new QLabel("You");
    m_player1Name->setAlignment(Qt::AlignCenter);
    player1BoxLayout->addWidget(player1Label);
    player1BoxLayout->addWidget(m_player1Name);

    // Player 2 Box
    m_player2Box = new QWidget();
    m_player2Box->setObjectName("playerBox");
    m_player2Box->setFixedWidth(180);
    QVBoxLayout *player2BoxLayout = new QVBoxLayout(m_player2Box);
    QLabel *player2Label = new QLabel("Player 2 (O)");
    player2Label->setAlignment(Qt::AlignCenter);
    m_player2Name = new QLabel("AI");
    m_player2Name->setAlignment(Qt::AlignCenter);
    player2BoxLayout->addWidget(player2Label);
    player2BoxLayout->addWidget(m_player2Name);

    playerInfoLayout->addWidget(m_player1Box);
    playerInfoLayout->addWidget(m_player2Box);
    leftSideLayout->addLayout(playerInfoLayout);

    // AI Difficulty panel
    m_difficultyContainer = new QWidget();
    m_difficultyContainer->setObjectName("difficultyContainer");
    QVBoxLayout *difficultyLayout = new QVBoxLayout(m_difficultyContainer);

    QLabel *difficultyTitle = new QLabel("AI Difficulty");
    difficultyTitle->setObjectName("panelTitle");
    difficultyTitle->setAlignment(Qt::AlignCenter);
    difficultyLayout->addWidget(difficultyTitle);

    // Difficulty buttons in 2 rows: Easy, Medium, Hard in first row, Expert in second row
    QGridLayout *difficultyButtonsGrid = new QGridLayout();
    difficultyButtonsGrid->setSpacing(10);

    m_easyBtn = new QPushButton("Easy");
    m_easyBtn->setObjectName("difficultyButton");
    m_easyBtn->setProperty("selected", false);
    
    m_mediumBtn = new QPushButton("Medium");
    m_mediumBtn->setObjectName("difficultyButton");
    m_mediumBtn->setProperty("selected", true);
    m_mediumBtn->setStyleSheet("background-color: rgba(0, 255, 255, 0.3); border: 2px solid #00ffff; color: white; font-weight: bold; text-shadow: 0 0 5px #00ffff, 0 0 8px #00ffff; box-shadow: 0 0 10px rgba(0, 255, 255, 0.8);");
    
    m_hardBtn = new QPushButton("Hard");
    m_hardBtn->setObjectName("difficultyButton");
    m_hardBtn->setProperty("selected", false);
    
    m_expertBtn = new QPushButton("Expert");
    m_expertBtn->setObjectName("difficultyButton");
    m_expertBtn->setProperty("selected", false);

    difficultyButtonsGrid->addWidget(m_easyBtn, 0, 0);
    difficultyButtonsGrid->addWidget(m_mediumBtn, 0, 1);
    difficultyButtonsGrid->addWidget(m_hardBtn, 0, 2);
    difficultyButtonsGrid->addWidget(m_expertBtn, 1, 0);

    difficultyLayout->addLayout(difficultyButtonsGrid);
    leftSideLayout->addWidget(m_difficultyContainer);

    // Recent Games panel
    QWidget *historyPanel = new QWidget();
    historyPanel->setObjectName("historyPanel");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyPanel);

    QLabel *historyTitle = new QLabel("Recent Games");
    historyTitle->setObjectName("panelTitle");
    historyTitle->setAlignment(Qt::AlignCenter);
    historyLayout->addWidget(historyTitle);

    m_historyList = new QListWidget();
    m_historyList->setObjectName("historyList");
    m_historyList->setMaximumHeight(150);
    historyLayout->addWidget(m_historyList);

    leftSideLayout->addWidget(historyPanel);
    leftSideLayout->addStretch();

    // Right side - game board
    QWidget *gameBoardContainer = new QWidget();
    QVBoxLayout *gameBoardLayout = new QVBoxLayout(gameBoardContainer);

    // Status message
    m_statusMessage = new QLabel("Your turn (X)");
    m_statusMessage->setObjectName("statusMessage");
    m_statusMessage->setAlignment(Qt::AlignCenter);
    m_statusMessage->setFixedHeight(40); // Reduced height
    gameBoardLayout->addWidget(m_statusMessage);

    // Game board grid
    QWidget *boardWidget = new QWidget();
    boardWidget->setObjectName("gameBoard");
    m_boardLayout = new QGridLayout(boardWidget);
    m_boardLayout->setSpacing(5); // Reduced spacing

    // Create cells
    m_cells.clear();
    for (int i = 0; i < 9; ++i) {
        QPushButton *cell = new QPushButton();
        cell->setObjectName("cellButton");
        cell->setProperty("index", i);
        cell->setFixedSize(90, 90); // Slightly smaller cells
        cell->setCursor(Qt::PointingHandCursor);

        m_cells.append(cell);
        m_boardLayout->addWidget(cell, i / 3, i % 3);

        connect(cell, &QPushButton::clicked, this, &MainWindow::onCellClicked);
    }

    gameBoardLayout->addWidget(boardWidget);

    // Game action buttons
    QHBoxLayout *gameActionsLayout = new QHBoxLayout();
    gameActionsLayout->setSpacing(10);

    m_newGameBtn = new QPushButton("New Game");
    m_saveGameBtn = new QPushButton("Save Game");
    m_exitGameBtn = new QPushButton("Exit Game");

    gameActionsLayout->addWidget(m_newGameBtn);
    gameActionsLayout->addWidget(m_saveGameBtn);
    gameActionsLayout->addWidget(m_exitGameBtn);

    gameBoardLayout->addLayout(gameActionsLayout);

    // Add left and right sides to the main container
    gameContainerLayout->addWidget(leftSideContainer);
    gameContainerLayout->addWidget(gameBoardContainer);

    // Add title and main container to the game screen
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(gameTitleLabel);
    mainLayout->addWidget(gameContainer);

    // Create toggle stats button - positioned at the bottom right
    m_toggleStatsViewBtn = new QPushButton("Toggle Statistics View");
    m_toggleStatsViewBtn->setObjectName("toggleStatsViewBtn");
    m_toggleStatsViewBtn->setFixedWidth(180); // Fixed width for the button

    // Create a container for the button with right alignment
    QHBoxLayout *statsButtonLayout = new QHBoxLayout();
    statsButtonLayout->addStretch();
    statsButtonLayout->addWidget(m_toggleStatsViewBtn);
    statsButtonLayout->setContentsMargins(0, 5, 10, 5); // Reduced margins

    mainLayout->addLayout(statsButtonLayout);

    // Add main layout to game screen
    gameLayout->addLayout(mainLayout);

    // Setup statistics view
    setupStatisticsView();
}

void MainWindow::setupStatisticsView() {
    // Statistics view should already be initialized in the constructor
    m_statisticsView->setObjectName("statisticsView");
    m_statisticsView->setGeometry(0, 0, this->width(), this->height());
    
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statisticsView);
    statsLayout->setContentsMargins(20, 20, 20, 20);
    statsLayout->setSpacing(20);

    QLabel *statsTitle = new QLabel("Player Statistics");
    statsTitle->setObjectName("titleLabel");
    statsTitle->setAlignment(Qt::AlignCenter);
    // Apply extra intense neon styling with white core
    statsTitle->setStyleSheet("color: #00eeff; text-shadow: 0 0 10px #00eeff, 0 0 20px #00eeff; font-weight: 800; letter-spacing: 3px; font-size: 48px; margin-bottom: 30px;");
    setupTitleAnimation(statsTitle);
    statsLayout->addWidget(statsTitle);

    // Create custom tab bar that looks like the HTML version
    QWidget *tabBarWidget = new QWidget();
    tabBarWidget->setObjectName("tabBar");
    QHBoxLayout *tabBarLayout = new QHBoxLayout(tabBarWidget);
    tabBarLayout->setSpacing(0);
    tabBarLayout->setContentsMargins(0, 0, 0, 0);
    tabBarLayout->setAlignment(Qt::AlignTop);
    
    // Create the tab buttons
    QPushButton *personalStatsTab = new QPushButton("My Stats");
    personalStatsTab->setObjectName("tabButton");
    personalStatsTab->setProperty("tabIndex", 0);
    personalStatsTab->setProperty("active", true);
    personalStatsTab->setStyleSheet("background-color: rgba(0, 255, 255, 0.2); box-shadow: 0 0 10px rgba(0, 255, 255, 0.5);");
    
    QPushButton *leaderboardTab = new QPushButton("Leaderboard");
    leaderboardTab->setObjectName("tabButton");
    leaderboardTab->setProperty("tabIndex", 1);
    leaderboardTab->setProperty("active", false);
    
    QPushButton *historyTab = new QPushButton("Full History");
    historyTab->setObjectName("tabButton");
    historyTab->setProperty("tabIndex", 2);
    historyTab->setProperty("active", false);
    
    // Add tabs to the tab bar
    tabBarLayout->addWidget(personalStatsTab);
    tabBarLayout->addWidget(leaderboardTab);
    tabBarLayout->addWidget(historyTab);
    
    // Add tab bar to main layout
    statsLayout->addWidget(tabBarWidget);
    
    // Create stacked widget for tab content
    QStackedWidget *tabContentWidget = new QStackedWidget();
    tabContentWidget->setObjectName("tabContent");
    
    // Connect tab buttons to switch pages
    connect(personalStatsTab, &QPushButton::clicked, [=]() {
        tabContentWidget->setCurrentIndex(0);
        personalStatsTab->setProperty("active", true);
        leaderboardTab->setProperty("active", false);
        historyTab->setProperty("active", false);
        personalStatsTab->setStyleSheet("background-color: rgba(0, 255, 255, 0.2); box-shadow: 0 0 10px rgba(0, 255, 255, 0.5);");
        leaderboardTab->setStyleSheet("");
        historyTab->setStyleSheet("");
    });
    
    connect(leaderboardTab, &QPushButton::clicked, [=]() {
        tabContentWidget->setCurrentIndex(1);
        personalStatsTab->setProperty("active", false);
        leaderboardTab->setProperty("active", true);
        historyTab->setProperty("active", false);
        personalStatsTab->setStyleSheet("");
        leaderboardTab->setStyleSheet("background-color: rgba(0, 255, 255, 0.2); box-shadow: 0 0 10px rgba(0, 255, 255, 0.5);");
        historyTab->setStyleSheet("");
        populateLeaderboard(); // Update leaderboard when switching to that tab
    });
    
    connect(historyTab, &QPushButton::clicked, [=]() {
        tabContentWidget->setCurrentIndex(2);
        personalStatsTab->setProperty("active", false);
        leaderboardTab->setProperty("active", false);
        historyTab->setProperty("active", true);
        personalStatsTab->setStyleSheet("");
        leaderboardTab->setStyleSheet("");
        historyTab->setStyleSheet("background-color: rgba(0, 255, 255, 0.2); box-shadow: 0 0 10px rgba(0, 255, 255, 0.5);");
    });
    
    // Personal Stats Tab Content
    QWidget *personalStatsContent = new QWidget();
    QVBoxLayout *personalStatsLayout = new QVBoxLayout(personalStatsContent);
    
    QWidget *statsPanel = new QWidget();
    statsPanel->setObjectName("statsPanel");
    QVBoxLayout *statsPanelLayout = new QVBoxLayout(statsPanel);
    
    QLabel *personalStatsTitle = new QLabel("Personal Statistics");
    personalStatsTitle->setObjectName("panelTitle");
    personalStatsTitle->setAlignment(Qt::AlignLeft);
    personalStatsTitle->setStyleSheet("color: #00eeff; font-size: 24px; font-weight: bold; text-shadow: 0 0 5px #00eeff; margin-bottom: 20px; padding-left: 0px;");
    statsPanelLayout->addWidget(personalStatsTitle);
    
    // Create grid layout for stats that looks like CSS Grid
    QGridLayout *statsGridLayout = new QGridLayout();
    statsGridLayout->setHorizontalSpacing(10);
    statsGridLayout->setVerticalSpacing(10);
    statsGridLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create stat boxes that match the HTML version
    auto createStatBox = [](const QString &label, QLabel *&valueLabel) {
        // Create a container with just the background and border
        QWidget *box = new QWidget();
        box->setObjectName("statBox");
        box->setStyleSheet("background-color: rgba(0, 136, 255, 0.1); border: 1px solid #0088ff; border-radius: 5px;");
        box->setMinimumHeight(80);
        
        // Create a single layout for the entire box
        QVBoxLayout *layout = new QVBoxLayout(box);
        layout->setSpacing(5);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->setAlignment(Qt::AlignCenter);
        
        // Create the label without any box or background
        QLabel *labelWidget = new QLabel(label);
        labelWidget->setAlignment(Qt::AlignCenter);
        labelWidget->setObjectName("statLabel");
        labelWidget->setStyleSheet("color: white; font-size: 16px; background: transparent; border: none;");
        
        // Create the value without any border or background
        valueLabel = new QLabel(label == "Win Rate" ? "0%" : "0");
        valueLabel->setAlignment(Qt::AlignCenter);
        valueLabel->setObjectName("statValue");
        valueLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #00eeff; background: transparent; border: none;");
        
        // Add widgets to layout
        layout->addWidget(labelWidget);
        layout->addWidget(valueLabel);
        
        return box;
    };
    
    QWidget *totalGamesBox = createStatBox("Total Games", m_statTotalGames);
    QWidget *winRateBox = createStatBox("Win Rate", m_statWinRate);
    QWidget *winsBox = createStatBox("Wins", m_statWins);
    QWidget *lossesBox = createStatBox("Losses", m_statLosses);
    QWidget *drawsBox = createStatBox("Draws", m_statDraws);
    QWidget *vsAIBox = createStatBox("vs AI", m_statVsAI);
    QWidget *vsPlayersBox = createStatBox("vs Players", m_statVsPlayers);
    QWidget *bestStreakBox = createStatBox("Best Streak", m_statBestStreak);
    
    // Add stat boxes to grid in a 2-column layout
    statsGridLayout->addWidget(totalGamesBox, 0, 0);
    statsGridLayout->addWidget(winRateBox, 0, 1);
    statsGridLayout->addWidget(winsBox, 1, 0);
    statsGridLayout->addWidget(lossesBox, 1, 1);
    statsGridLayout->addWidget(drawsBox, 2, 0);
    statsGridLayout->addWidget(vsAIBox, 2, 1);
    statsGridLayout->addWidget(vsPlayersBox, 3, 0);
    statsGridLayout->addWidget(bestStreakBox, 3, 1);
    
    statsPanelLayout->addLayout(statsGridLayout);
    personalStatsLayout->addWidget(statsPanel);
    
    // Leaderboard Tab Content
    QWidget *leaderboardContent = new QWidget();
    QVBoxLayout *leaderboardLayout = new QVBoxLayout(leaderboardContent);
    
    QWidget *leaderboardPanel = new QWidget();
    leaderboardPanel->setObjectName("statsPanel");
    QVBoxLayout *leaderboardPanelLayout = new QVBoxLayout(leaderboardPanel);
    
    QLabel *leaderboardTitle = new QLabel("Global Leaderboard");
    leaderboardTitle->setObjectName("panelTitle");
    leaderboardTitle->setAlignment(Qt::AlignCenter);
    leaderboardPanelLayout->addWidget(leaderboardTitle);
    
    m_leaderboardList = new QListWidget();
    m_leaderboardList->setObjectName("leaderboardList");
    m_leaderboardList->setStyleSheet("background-color: transparent; border: none;");
    leaderboardPanelLayout->addWidget(m_leaderboardList);
    leaderboardLayout->addWidget(leaderboardPanel);
    
    // Game History Tab Content
    QWidget *historyContent = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyContent);
    
    QWidget *fullHistoryPanel = new QWidget();
    fullHistoryPanel->setObjectName("statsPanel");
    QVBoxLayout *fullHistoryLayout = new QVBoxLayout(fullHistoryPanel);
    
    QLabel *fullHistoryTitle = new QLabel("Complete Game History");
    fullHistoryTitle->setObjectName("panelTitle");
    fullHistoryTitle->setAlignment(Qt::AlignCenter);
    fullHistoryLayout->addWidget(fullHistoryTitle);
    
    m_fullHistoryList = new QListWidget();
    m_fullHistoryList->setObjectName("fullHistoryList");
    m_fullHistoryList->setStyleSheet("background-color: transparent; border: none;");
    fullHistoryLayout->addWidget(m_fullHistoryList);
    historyLayout->addWidget(fullHistoryPanel);
    
    // Add content pages to stacked widget
    tabContentWidget->addWidget(personalStatsContent);
    tabContentWidget->addWidget(leaderboardContent);
    tabContentWidget->addWidget(historyContent);
    
    // Add stacked widget to main layout
    statsLayout->addWidget(tabContentWidget);
    
    // Add back button at the bottom
    m_backToGameBtn = new QPushButton("Back to Game");
    m_backToGameBtn->setObjectName("backToGameBtn");
    m_backToGameBtn->setFixedHeight(40);
    
    // Create a container for the back button with proper positioning
    QHBoxLayout *backBtnLayout = new QHBoxLayout();
    backBtnLayout->addWidget(m_backToGameBtn);
    backBtnLayout->setContentsMargins(0, 20, 0, 0);
    
    statsLayout->addLayout(backBtnLayout);
    
    // Initially hide statistics view
    m_statisticsView->hide();
}

void MainWindow::setupConnections() {
    // Game logic connections
    connect(m_gameLogic, &GameLogic::gameOver, this, &MainWindow::onGameOver);
    connect(m_gameLogic, &GameLogic::playerChanged, this, &MainWindow::onPlayerChanged);
    connect(m_gameLogic, &GameLogic::boardChanged, this, &MainWindow::onBoardChanged);

    // Button connections
    connect(m_vsAIBtn, &QPushButton::clicked, this, &MainWindow::onVsAIClicked);
    connect(m_vsPlayerBtn, &QPushButton::clicked, this, &MainWindow::onVsPlayerClicked);
    connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(m_startPvpGameBtn, &QPushButton::clicked, this, &MainWindow::onStartPvpGameClicked);
    connect(m_backToModeBtn, &QPushButton::clicked, this, &MainWindow::onBackToModeClicked);
    connect(m_newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(m_saveGameBtn, &QPushButton::clicked, this, &MainWindow::onSaveGameClicked);
    connect(m_exitGameBtn, &QPushButton::clicked, this, &MainWindow::onExitGameClicked);
    connect(m_toggleStatsViewBtn, &QPushButton::clicked, this, &MainWindow::onToggleStatsViewClicked);
    connect(m_backToGameBtn, &QPushButton::clicked, this, &MainWindow::onBackToGameClicked);

    // Difficulty buttons
    connect(m_easyBtn, &QPushButton::clicked, this, &MainWindow::onDifficultyButtonClicked);
    connect(m_mediumBtn, &QPushButton::clicked, this, &MainWindow::onDifficultyButtonClicked);
    connect(m_hardBtn, &QPushButton::clicked, this, &MainWindow::onDifficultyButtonClicked);
    connect(m_expertBtn, &QPushButton::clicked, this, &MainWindow::onDifficultyButtonClicked);

    // Tab connections have been replaced with direct button connections in setupStatisticsView

    // Connect cells to click handler
    const int cellCount = m_cells.size();
    for (int i = 0; i < cellCount; ++i) {
        QPushButton* cell = m_cells.at(i);
        connect(cell, &QPushButton::clicked, this, &MainWindow::onCellClicked);
    }
}

void MainWindow::setupStyleSheet() {
    // Apply stylesheet to the parent widget - not needed since we load it in main.cpp
}

void MainWindow::setupTitleAnimation(QLabel* titleLabel) {
    // Apply direct styling without animation for now
    // Create an extra intense neon effect with white core for maximum brightness
    QString neonStyle = "color: #00ffff; text-shadow: 0 0 2px #fff, 0 0 4px #fff, 0 0 6px #fff, 0 0 8px #fff, "
                       "0 0 10px #00ffff, 0 0 15px #00ffff, 0 0 20px #00ffff, 0 0 25px #00ffff, "
                       "0 0 30px #00ffff, 0 0 35px #00ffff, 0 0 40px #00ffff, 0 0 50px #00ffff, "
                       "0 0 60px #00ffff, 0 0 70px #00ffff; "
                       "font-weight: 800; letter-spacing: 3px; font-size: 42px;";
    
    titleLabel->setStyleSheet(neonStyle);
}

void MainWindow::showScreen(Screen screen) {
    // Show loading before changing screen
    showLoading();

    switch (screen) {
    case Screen::ModeSelection:
        m_stackedWidget->setCurrentWidget(m_modeSelectionScreen);
        m_statisticsView->hide();
        m_toggleStatsViewBtn->hide();
        m_backToGameBtn->hide();
        break;

    case Screen::PlayerAuth:
        m_stackedWidget->setCurrentWidget(m_playerAuthScreen);
        m_statisticsView->hide();
        m_toggleStatsViewBtn->hide();
        m_backToGameBtn->hide();
        break;

    case Screen::Game:
        m_stackedWidget->setCurrentWidget(m_gameScreen);
        m_statisticsView->hide();
        m_toggleStatsViewBtn->show();
        m_backToGameBtn->hide();

        if (m_gameMode == GameMode::AI) {
            m_difficultyContainer->show();
        } else {
            m_difficultyContainer->hide();
        }
        break;

    case Screen::Statistics:
        m_stackedWidget->setCurrentWidget(m_gameScreen);
        m_statisticsView->show();
        m_toggleStatsViewBtn->hide();
        m_backToGameBtn->show();
        break;
    }
}

void MainWindow::onCellClicked() {
    // Check if user is logged in
    if (!m_auth->getCurrentUser()) {
        QMessageBox::warning(this, "Authentication Required", "Please login first to play the game.");
        return;
    }

    QPushButton* cell = qobject_cast<QPushButton*>(sender());
    if (!cell) return;

    int index = cell->property("index").toInt();

    if (m_gameLogic->makeMove(index) && m_gameMode == GameMode::AI) {
        // AI should make a move after a short delay
        QTimer::singleShot(700, m_aiOpponent, &AIOpponent::makeMove);
    }
}

void MainWindow::onNewGameClicked() {
    resetGame();
}

void MainWindow::onSaveGameClicked() {
    QMessageBox::information(this, "Save Game", "Game saved successfully!");
}

void MainWindow::onExitGameClicked() {
    showScreen(Screen::ModeSelection);
}

void MainWindow::onVsAIClicked() {
    if (!m_auth->getCurrentUser()) {
        m_loginStatus->setText("Please login first");
        m_loginStatus->setProperty("status", "error");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
        return;
    }

    m_gameMode = GameMode::AI;
    showScreen(Screen::Game);

    // Set player names
    m_player1Name->setText(m_auth->getCurrentUser()->getUsername());
    m_player2Name->setText("AI");

    // Show AI difficulty panel
    m_difficultyContainer->show();

    // Set medium difficulty as default (matching HTML version)
    m_easyBtn->setProperty("selected", false);
    m_mediumBtn->setProperty("selected", true);
    m_hardBtn->setProperty("selected", false);
    m_expertBtn->setProperty("selected", false);

    // Update button styles
    m_easyBtn->style()->unpolish(m_easyBtn);
    m_easyBtn->style()->polish(m_easyBtn);
    m_easyBtn->update();

    m_mediumBtn->style()->unpolish(m_mediumBtn);
    m_mediumBtn->style()->polish(m_mediumBtn);
    m_mediumBtn->update();

    m_hardBtn->style()->unpolish(m_hardBtn);
    m_hardBtn->style()->polish(m_hardBtn);
    m_hardBtn->update();

    m_expertBtn->style()->unpolish(m_expertBtn);
    m_expertBtn->style()->polish(m_expertBtn);
    m_expertBtn->update();

    // Set game difficulty to medium (default)
    m_gameLogic->setAIDifficulty(GameLogic::AIDifficulty::Medium);

    // Reset the game
    resetGame();

    // Show loading animation
    showLoading();
}

void MainWindow::onVsPlayerClicked() {
    if (!m_auth->getCurrentUser()) {
        m_loginStatus->setText("Please login first");
        m_loginStatus->setProperty("status", "error");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
        return;
    }

    m_player1UsernameInput->setText(m_auth->getCurrentUser()->getUsername());
    m_player1PasswordInput->clear();
    m_player2UsernameInput->clear();
    m_player2PasswordInput->clear();

    showScreen(Screen::PlayerAuth);
}

void MainWindow::onDifficultyButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    // Reset all buttons to default style
    QString defaultStyle = "background-color: transparent; border: 1px solid #00ffff; color: #00ffff;";
    m_easyBtn->setStyleSheet(defaultStyle);
    m_mediumBtn->setStyleSheet(defaultStyle);
    m_hardBtn->setStyleSheet(defaultStyle);
    m_expertBtn->setStyleSheet(defaultStyle);
    
    // Remove selected state from all difficulty buttons
    m_easyBtn->setProperty("selected", false);
    m_mediumBtn->setProperty("selected", false);
    m_hardBtn->setProperty("selected", false);
    m_expertBtn->setProperty("selected", false);

    // Set selected state for clicked button
    button->setProperty("selected", true);
    
    // Apply direct styling to the selected button for immediate visual feedback
    QString selectedStyle = "background-color: rgba(0, 255, 255, 0.3); border: 2px solid #00ffff; color: white; font-weight: bold; text-shadow: 0 0 5px #00ffff, 0 0 8px #00ffff; box-shadow: 0 0 10px rgba(0, 255, 255, 0.8);";
    button->setStyleSheet(selectedStyle);

    // Set game difficulty
    GameLogic::AIDifficulty difficulty;
    QString difficultyName;

    if (button == m_easyBtn) {
        difficulty = GameLogic::AIDifficulty::Easy;
        difficultyName = "Easy";
    } else if (button == m_mediumBtn) {
        difficulty = GameLogic::AIDifficulty::Medium;
        difficultyName = "Medium";
    } else if (button == m_hardBtn) {
        difficulty = GameLogic::AIDifficulty::Hard;
        difficultyName = "Hard";
    } else if (button == m_expertBtn) {
        difficulty = GameLogic::AIDifficulty::Expert;
        difficultyName = "Expert";
    } else {
        difficulty = GameLogic::AIDifficulty::Medium;
        difficultyName = "Medium";
    }

    m_gameLogic->setAIDifficulty(difficulty);

    // If we're in the middle of a game, show a message about difficulty change
    if (m_gameMode == GameMode::AI && m_gameLogic->getGameResult() != GameLogic::GameResult::InProgress) {
        QMessageBox::information(this, "Difficulty Changed",
                                 QString("AI difficulty set to %1.\nChanges will take effect in the next game.").arg(difficultyName));
    }
}

void MainWindow::onLoginClicked() {
    QString username = m_usernameInput->text();
    QString password = m_passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_loginStatus->setText("Please enter both username and password");
        m_loginStatus->setProperty("status", "error");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
        return;
    }

    if (m_auth->login(username, password)) {
        m_loginStatus->setText(QString("Logged in as %1").arg(username));
        m_loginStatus->setProperty("status", "success");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
    } else {
        m_loginStatus->setText("Invalid username or password");
        m_loginStatus->setProperty("status", "error");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
    }
}

void MainWindow::onRegisterClicked() {
    QString username = m_usernameInput->text();
    QString password = m_passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_loginStatus->setText("Please enter both username and password");
        m_loginStatus->setProperty("status", "error");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
        return;
    }

    if (m_auth->registerUser(username, password)) {
        m_loginStatus->setText("Registration successful. You can now login.");
        m_loginStatus->setProperty("status", "success");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
    } else {
        m_loginStatus->setText("Username already exists");
        m_loginStatus->setProperty("status", "error");
        m_loginStatus->style()->unpolish(m_loginStatus);
        m_loginStatus->style()->polish(m_loginStatus);
    }
}

void MainWindow::onStartPvpGameClicked() {
    QString player1Username = m_player1UsernameInput->text();
    QString player1Password = m_player1PasswordInput->text();
    QString player2Username = m_player2UsernameInput->text();
    QString player2Password = m_player2PasswordInput->text();

    QString errorMessage;
    if (m_auth->authenticatePlayers(player1Username, player1Password,
                                    player2Username, player2Password,
                                    errorMessage)) {
        m_gameMode = GameMode::Player;
        m_player1User = player1Username;
        m_player2User = player2Username;

        showScreen(Screen::Game);

        m_player1Name->setText(player1Username);
        m_player2Name->setText(player2Username);

        resetGame();
    } else {
        QMessageBox::warning(this, "Authentication Error", errorMessage);
    }
}

void MainWindow::onBackToModeClicked() {
    showScreen(Screen::ModeSelection);
}

void MainWindow::onToggleStatsViewClicked() {
    showScreen(Screen::Statistics);
    updateStatistics();
}

void MainWindow::onBackToGameClicked() {
    showScreen(Screen::Game);
}

void MainWindow::onTabClicked(int index) {
    if (index == 1) { // Leaderboard tab
        populateLeaderboard();
    }
}

void MainWindow::onGameOver(GameLogic::GameResult result) {
    switch (result) {
    case GameLogic::GameResult::XWins:
        if (m_gameMode == GameMode::AI) {
            updateGameStatus("You win!");
            if (m_auth->getCurrentUser()) {
                m_auth->getCurrentUser()->addGame("win", "ai",
                                                  m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy ? "easy" :
                                                      m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium ? "medium" :
                                                      m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Hard ? "hard" : "expert");
            }

            // Get difficulty name for history
            QString difficultyName =
                m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy ? "easy" :
                    m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium ? "medium" :
                    m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Hard ? "hard" : "expert";

            addGameToHistory(QString("Game vs AI (%1) - Win").arg(difficultyName));
        } else {
            updateGameStatus(QString("%1 wins!").arg(m_player1User));

            User* player1 = nullptr;
            User* player2 = nullptr;

            const auto& users = m_auth->getUsers();
            for (User* user : users) {
                if (user->getUsername() == m_player1User) {
                    player1 = user;
                }
                if (user->getUsername() == m_player2User) {
                    player2 = user;
                }
            }

            if (player1) {
                player1->addGame("win", m_player2User);
            }

            if (player2) {
                player2->addGame("loss", m_player1User);
            }

            addGameToHistory(QString("%1 (X) defeated %2").arg(m_player1User, m_player2User));
        }
        break;

    case GameLogic::GameResult::OWins:
        if (m_gameMode == GameMode::AI) {
            updateGameStatus("AI wins!");
            if (m_auth->getCurrentUser()) {
                m_auth->getCurrentUser()->addGame("loss", "ai",
                                                  m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy ? "easy" :
                                                      m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium ? "medium" :
                                                      m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Hard ? "hard" : "expert");
            }

            // Get difficulty name for history
            QString difficultyName =
                m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy ? "easy" :
                    m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium ? "medium" :
                    m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Hard ? "hard" : "expert";

            addGameToHistory(QString("Game vs AI (%1) - Loss").arg(difficultyName));
        } else {
            updateGameStatus(QString("%1 wins!").arg(m_player2User));

            User* player1 = nullptr;
            User* player2 = nullptr;

            const auto& users = m_auth->getUsers();
            for (User* user : users) {
                if (user->getUsername() == m_player1User) {
                    player1 = user;
                }
                if (user->getUsername() == m_player2User) {
                    player2 = user;
                }
            }

            if (player1) {
                player1->addGame("loss", m_player2User);
            }

            if (player2) {
                player2->addGame("win", m_player1User);
            }

            addGameToHistory(QString("%1 (O) defeated %2").arg(m_player2User, m_player1User));
        }
        break;

    case GameLogic::GameResult::Draw:
        updateGameStatus("It's a draw!");

        if (m_gameMode == GameMode::AI) {
            if (m_auth->getCurrentUser()) {
                m_auth->getCurrentUser()->addGame("draw", "ai",
                                                  m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy ? "easy" :
                                                      m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium ? "medium" :
                                                      m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Hard ? "hard" : "expert");
            }

            // Get difficulty name for history
            QString difficultyName =
                m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Easy ? "easy" :
                    m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Medium ? "medium" :
                    m_gameLogic->getAIDifficulty() == GameLogic::AIDifficulty::Hard ? "hard" : "expert";

            addGameToHistory(QString("Game vs AI (%1) - Draw").arg(difficultyName));
        } else {
            User* player1 = nullptr;
            User* player2 = nullptr;

            const auto& users = m_auth->getUsers();
            for (User* user : users) {
                if (user->getUsername() == m_player1User) {
                    player1 = user;
                }
                if (user->getUsername() == m_player2User) {
                    player2 = user;
                }
            }

            if (player1) {
                player1->addGame("draw", m_player2User);
            }

            if (player2) {
                player2->addGame("draw", m_player1User);
            }

            addGameToHistory(QString("Draw between %1 and %2").arg(m_player1User, m_player2User));
        }
        break;

    default:
        break;
    }

    highlightWinningCells();

    // Update statistics if we're viewing them
    if (m_statisticsView->isVisible()) {
        updateStatistics();
    }
}

void MainWindow::onPlayerChanged(GameLogic::Player player) {
    if (m_gameMode == GameMode::AI) {
        if (player == GameLogic::Player::X) {
            updateGameStatus("Your turn (X)");
            m_player1Box->setProperty("current-player", true);
            m_player2Box->setProperty("current-player", false);
        } else {
            updateGameStatus("AI's turn (O)");
            m_player1Box->setProperty("current-player", false);
            m_player2Box->setProperty("current-player", true);
        }
    } else {
        if (player == GameLogic::Player::X) {
            updateGameStatus(QString("%1's turn (X)").arg(m_player1User));
            m_player1Box->setProperty("current-player", true);
            m_player2Box->setProperty("current-player", false);
        } else {
            updateGameStatus(QString("%1's turn (O)").arg(m_player2User));
            m_player1Box->setProperty("current-player", false);
            m_player2Box->setProperty("current-player", true);
        }
    }

    // Update style
    m_player1Box->style()->unpolish(m_player1Box);
    m_player1Box->style()->polish(m_player1Box);
    m_player2Box->style()->unpolish(m_player2Box);
    m_player2Box->style()->polish(m_player2Box);
}

void MainWindow::onBoardChanged() {
    for (int i = 0; i < 9; ++i) {
        auto state = m_gameLogic->getCellState(i);

        if (state == GameLogic::Player::X) {
            m_cells[i]->setText("X");
            m_cells[i]->setProperty("cell-value", "X");
            // Explicitly set text color to match the HTML version
            m_cells[i]->setStyleSheet("color: #ff69b4; text-shadow: 0 0 10px #ff69b4;");
        } else if (state == GameLogic::Player::O) {
            m_cells[i]->setText("O");
            m_cells[i]->setProperty("cell-value", "O");
            // Explicitly set text color to match the HTML version
            m_cells[i]->setStyleSheet("color: #00ffff; text-shadow: 0 0 10px #00ffff;");
        } else {
            m_cells[i]->setText("");
            m_cells[i]->setProperty("cell-value", "");
            m_cells[i]->setStyleSheet("");
        }

        m_cells[i]->style()->unpolish(m_cells[i]);
        m_cells[i]->style()->polish(m_cells[i]);
    }
}

void MainWindow::updateGameStatus(const QString &message) {
    m_statusMessage->setText(message);
}

void MainWindow::updatePlayerInfo() {
    if (m_gameMode == GameMode::AI) {
        m_player1Name->setText(m_auth->getCurrentUser() ? m_auth->getCurrentUser()->getUsername() : "You");
        m_player2Name->setText("AI");
    } else {
        m_player1Name->setText(m_player1User);
        m_player2Name->setText(m_player2User);
    }
}

void MainWindow::updateStatistics() {
    User* currentUser = m_auth->getCurrentUser();
    if (!currentUser) return;

    m_statTotalGames->setText(QString::number(currentUser->getTotalGames()));
    m_statWins->setText(QString::number(currentUser->getWins()));
    m_statLosses->setText(QString::number(currentUser->getLosses()));
    m_statDraws->setText(QString::number(currentUser->getDraws()));
    m_statVsAI->setText(QString::number(currentUser->getVsAI()));
    m_statVsPlayers->setText(QString::number(currentUser->getVsPlayers()));
    m_statWinRate->setText(QString("%1%").arg(currentUser->getWinRate()));
    m_statBestStreak->setText(QString::number(currentUser->getBestStreak()));

    // Update full history
    m_fullHistoryList->clear();
    const auto& gameHistory = currentUser->getGameHistory();
    for (const GameRecord &record : gameHistory) {
        m_fullHistoryList->addItem(QString("%1 - %2").arg(record.date, record.result));
    }
}

void MainWindow::populateLeaderboard() {
    m_leaderboardList->clear();

    auto leaderboard = m_database->getLeaderboard();

    if (leaderboard.isEmpty()) {
        // Add dummy data if empty - formatted exactly like the HTML version
        QListWidgetItem* item1 = new QListWidgetItem();
        item1->setText("1. Player123");
        item1->setData(Qt::UserRole, "Wins: 28 | Win Rate: 85%");
        m_leaderboardList->addItem(item1);

        QListWidgetItem* item2 = new QListWidgetItem();
        item2->setText("2. GameMaster");
        item2->setData(Qt::UserRole, "Wins: 24 | Win Rate: 80%");
        m_leaderboardList->addItem(item2);

        QListWidgetItem* item3 = new QListWidgetItem();
        item3->setText("3. TicTacPro");
        item3->setData(Qt::UserRole, "Wins: 19 | Win Rate: 76%");
        m_leaderboardList->addItem(item3);
    } else {
        for (int i = 0; i < leaderboard.size(); ++i) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(QString::number(i + 1) + ". " + leaderboard[i].first);
            item->setData(Qt::UserRole, leaderboard[i].second);
            m_leaderboardList->addItem(item);
        }
    }
    
    // Set item height to match HTML version
    for (int i = 0; i < m_leaderboardList->count(); ++i) {
        QListWidgetItem* item = m_leaderboardList->item(i);
        item->setSizeHint(QSize(item->sizeHint().width(), 50)); // Set height to 50px
    }
    
    // If we haven't already set the delegate, set it now
    if (!m_leaderboardDelegate) {
        m_leaderboardDelegate = new LeaderboardItemDelegate();
        m_leaderboardList->setItemDelegate(m_leaderboardDelegate);
    }
}

void MainWindow::highlightWinningCells() {
    const QVector<int> winPattern = m_gameLogic->getWinPattern();

    // Clear all win states first
    const int cellCount = m_cells.size();
    for (int i = 0; i < cellCount; ++i) {
        QPushButton* cell = m_cells.at(i);
        cell->setProperty("winner", false);
        cell->style()->unpolish(cell);
        cell->style()->polish(cell);
    }

    // Set win state for winning cells
    const auto& pattern = winPattern;
    for (const int& index : pattern) {
        m_cells[index]->setProperty("winner", true);
        m_cells[index]->style()->unpolish(m_cells[index]);
        m_cells[index]->style()->polish(m_cells[index]);
    }
}

void MainWindow::addGameToHistory(const QString &result) {
    m_gameHistory->addEntry(result);

    m_historyList->clear();
    const auto& entries = m_gameHistory->getEntries();
    for (const GameHistory::HistoryEntry &entry : entries) {
        m_historyList->addItem(entry.result);
    }
}

void MainWindow::resetGame() {
    m_gameLogic->resetBoard();

    updatePlayerInfo();

    // Set the correct player indicators based on game mode
    if (m_gameMode == GameMode::AI) {
        // In AI mode, player 1 (human) always starts
        m_player1Box->setProperty("current-player", true);
        m_player2Box->setProperty("current-player", false);
        updateGameStatus("Your turn (X)");
    } else {
        // In player vs player mode, player 1 starts
        m_player1Box->setProperty("current-player", true);
        m_player2Box->setProperty("current-player", false);
        updateGameStatus(QString("%1's turn (X)").arg(m_player1User));
    }

    // Update style
    m_player1Box->style()->unpolish(m_player1Box);
    m_player1Box->style()->polish(m_player1Box);
    m_player2Box->style()->unpolish(m_player2Box);
    m_player2Box->style()->polish(m_player2Box);

    // Clear any winning cell highlights
    const int cellCount = m_cells.size();
    for (int i = 0; i < cellCount; ++i) {
        QPushButton* cell = m_cells.at(i);
        cell->setProperty("winner", false);
        cell->style()->unpolish(cell);
        cell->style()->polish(cell);
    }
}

void MainWindow::showLoading(int duration) {
    m_loadingOverlay->setVisible(true);
    m_loadingOverlay->raise();

    // Hide overlay after duration
    QTimer::singleShot(duration, this, [this]() {
        m_loadingOverlay->setVisible(false);
    });
}
