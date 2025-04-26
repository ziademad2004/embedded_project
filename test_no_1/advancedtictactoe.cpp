#include "advancedtictactoe.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>

AdvancedTicTacToe::AdvancedTicTacToe(QWidget *parent)
    : QMainWindow(parent), playerXTurn(true), gameEnded(false),
    currentDifficulty("Medium"), playingAgainstAI(true), isSinglePlayer(true)
{
    board = QVector<int>(9, 0); // Initialize with 9 empty cells
    setupUI();
    setupStyleSheet();
    loadUserData();
}

AdvancedTicTacToe::~AdvancedTicTacToe()
{
    saveUserData();
}

void AdvancedTicTacToe::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Set window title and size
    setWindowTitle("Advanced Tic Tac Toe");
    resize(800, 600);

    // Create a stacked widget to switch between auth and game pages
    mainStack = new QStackedWidget(this);

    // Create authentication page
    authPage = new QWidget();
    QVBoxLayout *authLayout = new QVBoxLayout(authPage);

    QLabel *titleLabel = new QLabel("Advanced Tic Tac Toe");
    QFont titleFont("Arial", 28, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    authLayout->addWidget(titleLabel);

    // User auth group box
    QGroupBox *authGroup = new QGroupBox("User Authentication");
    QVBoxLayout *authGroupLayout = new QVBoxLayout(authGroup);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Username");
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QHBoxLayout *authButtonsLayout = new QHBoxLayout();
    loginButton = new QPushButton("Login");
    registerButton = new QPushButton("Register");
    authButtonsLayout->addWidget(loginButton);
    authButtonsLayout->addWidget(registerButton);

    authGroupLayout->addWidget(usernameEdit);
    authGroupLayout->addWidget(passwordEdit);
    authGroupLayout->addLayout(authButtonsLayout);

    authLayout->addWidget(authGroup);

    // Create game page
    gamePage = new QWidget();
    QHBoxLayout *gameLayout = new QHBoxLayout(gamePage);

    // Left panel for user info and game history
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();

    QLabel *userInfoLabel = new QLabel("Welcome, Guest");
    userInfoLabel->setObjectName("userInfoLabel");
    leftPanelLayout->addWidget(userInfoLabel);

    QGroupBox *historyGroup = new QGroupBox("Game History");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);

    gameHistoryList = new QListWidget();
    historyLayout->addWidget(gameHistoryList);

    leftPanelLayout->addWidget(historyGroup);

    // Right panel for game board and controls
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();

    statusLabel = new QLabel("Player X's turn");
    statusLabel->setAlignment(Qt::AlignCenter);
    QFont statusFont = statusLabel->font();
    statusFont.setPointSize(16);
    statusLabel->setFont(statusFont);
    rightPanelLayout->addWidget(statusLabel);

    // Grid for the cells
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(2);

    // Create the game board
    QFrame *boardFrame = new QFrame();
    boardFrame->setObjectName("gameBoard");
    boardFrame->setFrameShape(QFrame::StyledPanel);
    boardFrame->setLayout(gridLayout);

    // Create 9 buttons for the 3x3 grid
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            QPushButton *button = new QPushButton("");
            button->setFixedSize(100, 100);
            button->setObjectName("cellButton");
            QFont cellFont = button->font();
            cellFont.setPointSize(32);
            button->setFont(cellFont);
            button->setProperty("row", row);
            button->setProperty("col", col);
            button->setProperty("index", row * 3 + col);

            connect(button, &QPushButton::clicked, this, &AdvancedTicTacToe::cellClicked);

            gridLayout->addWidget(button, row, col);
            cells.push_back(button);
        }
    }

    rightPanelLayout->addWidget(boardFrame);

    // Game control buttons
    QHBoxLayout *gameControlLayout = new QHBoxLayout();
    newGameButton = new QPushButton("New Game");
    saveGameButton = new QPushButton("Save Game");
    gameControlLayout->addWidget(newGameButton);
    gameControlLayout->addWidget(saveGameButton);
    rightPanelLayout->addLayout(gameControlLayout);

    // Difficulty selector
    QGroupBox *difficultyGroup = new QGroupBox("AI Difficulty");
    QHBoxLayout *difficultyLayout = new QHBoxLayout(difficultyGroup);

    easyButton = new QPushButton("Easy");
    mediumButton = new QPushButton("Medium");
    hardButton = new QPushButton("Hard");
    expertButton = new QPushButton("Expert");

    difficultyLayout->addWidget(easyButton);
    difficultyLayout->addWidget(mediumButton);
    difficultyLayout->addWidget(hardButton);
    difficultyLayout->addWidget(expertButton);

    rightPanelLayout->addWidget(difficultyGroup);

    // Add left and right panels to game layout
    gameLayout->addLayout(leftPanelLayout, 1);
    gameLayout->addLayout(rightPanelLayout, 2);

    // Add pages to stack
    mainStack->addWidget(authPage);
    mainStack->addWidget(gamePage);

    // Main layout
    mainLayout->addWidget(mainStack);

    // Connect signals
    connect(loginButton, &QPushButton::clicked, this, &AdvancedTicTacToe::login);
    connect(registerButton, &QPushButton::clicked, this, &AdvancedTicTacToe::registerUser);
    connect(newGameButton, &QPushButton::clicked, this, &AdvancedTicTacToe::newGame);
    connect(saveGameButton, &QPushButton::clicked, this, &AdvancedTicTacToe::saveGame);

    connect(easyButton, &QPushButton::clicked, [this]() { setDifficulty("Easy"); });
    connect(mediumButton, &QPushButton::clicked, [this]() { setDifficulty("Medium"); });
    connect(hardButton, &QPushButton::clicked, [this]() { setDifficulty("Hard"); });
    connect(expertButton, &QPushButton::clicked, [this]() { setDifficulty("Expert"); });

    // Start with auth page
    mainStack->setCurrentWidget(authPage);

    // Apply neon effects
    applyNeonEffect(titleLabel, QColor(0, 200, 255));
    applyNeonEffect(loginButton, QColor(0, 180, 255));
    applyNeonEffect(registerButton, QColor(0, 180, 255));
    applyNeonEffect(newGameButton, QColor(0, 180, 255));
    applyNeonEffect(saveGameButton, QColor(0, 180, 255));
    applyNeonEffect(easyButton, QColor(0, 180, 255));
    applyNeonEffect(mediumButton, QColor(0, 180, 255));
    applyNeonEffect(hardButton, QColor(0, 180, 255));
    applyNeonEffect(expertButton, QColor(0, 180, 255));
}

void AdvancedTicTacToe::setupStyleSheet()
{
    // Create dark neon theme
    QString styleSheet = R"(
        QWidget {
            background-color: #050a18;
            color: #00b4ff;
            font-family: Arial;
        }

        QLabel {
            color: #00b4ff;
        }

        QPushButton {
            background-color: #0a1a33;
            color: #00b4ff;
            border: 1px solid #00b4ff;
            border-radius: 5px;
            padding: 5px;
            min-height: 30px;
        }

        QPushButton:hover {
            background-color: #102040;
        }

        QPushButton:pressed {
            background-color: #153055;
        }

        QLineEdit {
            background-color: #0a1a33;
            color: #00b4ff;
            border: 1px solid #00b4ff;
            border-radius: 5px;
            padding: 5px;
            selection-background-color: #00b4ff;
            selection-color: #050a18;
        }

        QGroupBox {
            border: 1px solid #00b4ff;
            border-radius: 5px;
            margin-top: 10px;
            font-weight: bold;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 5px;
        }

        #gameBoard {
            background-color: #0a1a33;
            border: 2px solid #00b4ff;
            border-radius: 10px;
        }

        #cellButton {
            background-color: transparent;
            border: 1px solid #00b4ff;
        }

        #cellButton[text="X"] {
            color: #ff5599;
        }

        #cellButton[text="O"] {
            color: #00b4ff;
        }

        QListWidget {
            background-color: #0a1a33;
            border: 1px solid #00b4ff;
            border-radius: 5px;
            color: #00b4ff;
        }

        QListWidget::item:selected {
            background-color: #00b4ff;
            color: #050a18;
        }
    )";

    setStyleSheet(styleSheet);
}

void AdvancedTicTacToe::applyNeonEffect(QWidget *widget, QColor color)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(15);
    effect->setColor(color);
    effect->setOffset(0, 0);
    widget->setGraphicsEffect(effect);
}

void AdvancedTicTacToe::login()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Error", "Please enter both username and password.");
        return;
    }

    // Check if user exists and password matches
    if (users.contains(username) && users[username] == password) {
        currentUser = username;
        QLabel *userInfoLabel = gamePage->findChild<QLabel*>("userInfoLabel");
        if (userInfoLabel) {
            userInfoLabel->setText("Welcome, " + currentUser);
        }

        // Load game history
        gameHistoryList->clear();
        if (gameHistory.contains(currentUser)) {
            gameHistoryList->addItems(gameHistory[currentUser]);
        }

        // Switch to game page
        mainStack->setCurrentWidget(gamePage);
        newGame();
    } else {
        QMessageBox::warning(this, "Login Error", "Invalid username or password.");
    }
}

void AdvancedTicTacToe::registerUser()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Registration Error", "Please enter both username and password.");
        return;
    }

    // Check if user already exists
    if (users.contains(username)) {
        QMessageBox::warning(this, "Registration Error", "Username already exists.");
        return;
    }

    // Register new user
    users[username] = password;
    gameHistory[username] = QStringList();
    saveUserData();

    QMessageBox::information(this, "Registration Success", "User registered successfully. You can now log in.");
}

void AdvancedTicTacToe::loadUserData()
{
    // In a real application, this would load from a database or file
    // For demonstration, we'll add some sample users
    if (users.isEmpty()) {
        users["user1"] = "password1";
        users["user2"] = "password2";

        gameHistory["user1"] = QStringList()
                               << "Game #1 vs AI (Expert) - Win"
                               << "Game #2 vs User2 - Loss"
                               << "Game #3 vs AI (Medium) - Tie";

        gameHistory["user2"] = QStringList()
                               << "Game #1 vs User1 - Win";
    }
}

void AdvancedTicTacToe::saveUserData()
{
    // In a real application, this would save to a database or file
    // This is just a placeholder
    qDebug() << "User data saved.";
}

void AdvancedTicTacToe::cellClicked()
{
    if (gameEnded) return;

    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int index = button->property("index").toInt();

    // Check if cell is already taken
    if (board[index] != 0 || button->text() != "") return;

    // Update the cell
    if (playerXTurn) {
        button->setText("X");
        board[index] = 1;
        statusLabel->setText("Player O's turn");
    } else {
        button->setText("O");
        board[index] = 2;
        statusLabel->setText("Player X's turn");
    }

    // Check game status
    checkGameStatus();

    // Switch player turn
    playerXTurn = !playerXTurn;

    // If playing against AI and not game ended, make AI move
    if (!gameEnded && playingAgainstAI && !playerXTurn) {
        QTimer::singleShot(500, this, &AdvancedTicTacToe::makeBotMove);
    }
}

void AdvancedTicTacToe::makeBotMove()
{
    if (gameEnded) return;

    QPoint move;

    // Get move based on difficulty
    if (currentDifficulty == "Easy") {
        move = getEasyMove();
    } else if (currentDifficulty == "Medium") {
        move = getMediumMove();
    } else if (currentDifficulty == "Hard") {
        move = getHardMove();
    } else { // Expert
        move = getExpertMove();
    }

    int index = move.x() * 3 + move.y();
    if (index >= 0 && index < 9 && board[index] == 0) {
        board[index] = 2; // AI is O
        cells[index]->setText("O");
        statusLabel->setText("Player X's turn");

        // Check game status
        checkGameStatus();

        // Switch player turn
        playerXTurn = !playerXTurn;
    }
}

QPoint AdvancedTicTacToe::getEasyMove()
{
    // Just make a random move
    QVector<int> availableMoves;

    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            availableMoves.append(i);
        }
    }

    if (availableMoves.isEmpty()) {
        return QPoint(-1, -1);
    }

    int randomIndex = QRandomGenerator::global()->bounded(availableMoves.size());
    int move = availableMoves[randomIndex];

    return QPoint(move / 3, move % 3);
}

QPoint AdvancedTicTacToe::getMediumMove()
{
    // 50% chance of making the best move, 50% random
    if (QRandomGenerator::global()->bounded(100) < 50) {
        return getBestMove();
    } else {
        return getEasyMove();
    }
}

QPoint AdvancedTicTacToe::getHardMove()
{
    // 80% chance of making the best move, 20% random
    if (QRandomGenerator::global()->bounded(100) < 80) {
        return getBestMove();
    } else {
        return getEasyMove();
    }
}

QPoint AdvancedTicTacToe::getExpertMove()
{
    // Always make the best move
    return getBestMove();
}

// Simple minimax algorithm for best move
QPoint AdvancedTicTacToe::getBestMove()
{
    // First, check if AI can win in one move
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            board[i] = 2; // Try placing O
            if (checkWin()) {
                board[i] = 0; // Reset
                return QPoint(i / 3, i % 3);
            }
            board[i] = 0; // Reset
        }
    }

    // Second, check if player can win in one move and block
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            board[i] = 1; // Try placing X
            if (checkWin()) {
                board[i] = 0; // Reset
                return QPoint(i / 3, i % 3);
            }
            board[i] = 0; // Reset
        }
    }

    // Try to take the center
    if (board[4] == 0) {
        return QPoint(1, 1);
    }

    // Try to take corners
    QVector<int> corners = {0, 2, 6, 8};
    QVector<int> availableCorners;

    for (int corner : corners) {
        if (board[corner] == 0) {
            availableCorners.append(corner);
        }
    }

    if (!availableCorners.isEmpty()) {
        int randomIndex = QRandomGenerator::global()->bounded(availableCorners.size());
        int move = availableCorners[randomIndex];
        return QPoint(move / 3, move % 3);
    }

    // Take any edge
    QVector<int> edges = {1, 3, 5, 7};
    QVector<int> availableEdges;

    for (int edge : edges) {
        if (board[edge] == 0) {
            availableEdges.append(edge);
        }
    }

    if (!availableEdges.isEmpty()) {
        int randomIndex = QRandomGenerator::global()->bounded(availableEdges.size());
        int move = availableEdges[randomIndex];
        return QPoint(move / 3, move % 3);
    }

    // Fallback to random move (should not reach here)
    return getEasyMove();
}

void AdvancedTicTacToe::checkGameStatus()
{
    // Check for win
    if (checkWin()) {
        QString winner = playerXTurn ? "X" : "O";
        statusLabel->setText("Player " + winner + " wins!");
        gameEnded = true;

        QString result;
        if (playingAgainstAI) {
            if ((winner == "X" && isSinglePlayer) || (winner == "O" && !isSinglePlayer)) {
                result = "Win";
            } else {
                result = "Loss";
            }
            updateGameHistory(QString("Game vs AI (%1) - %2").arg(currentDifficulty).arg(result));
        } else {
            result = winner == "X" ? "Win" : "Loss"; // Set result value
            QString opponent = winner == "X" ? "Player O" : "Player X";
            updateGameHistory(QString("Game vs %1 - %2").arg(opponent).arg(result));
        }

        QMessageBox::information(this, "Game Over", "Player " + winner + " wins!");
        return;
    }

    // Check for draw
    if (checkDraw()) {
        statusLabel->setText("Game ended in a draw!");
        gameEnded = true;

        QString opponent = playingAgainstAI ? QString("AI (%1)").arg(currentDifficulty) : "Player";
        updateGameHistory(QString("Game vs %1 - Tie").arg(opponent));

        QMessageBox::information(this, "Game Over", "Game ended in a draw!");
        return;
    }
}

bool AdvancedTicTacToe::checkWin()
{
    int currentPlayer = playerXTurn ? 1 : 2;

    // Check rows
    for (int row = 0; row < 3; row++) {
        if (board[row*3] == currentPlayer &&
            board[row*3+1] == currentPlayer &&
            board[row*3+2] == currentPlayer) {
            return true;
        }
    }

    // Check columns
    for (int col = 0; col < 3; col++) {
        if (board[col] == currentPlayer &&
            board[col+3] == currentPlayer &&
            board[col+6] == currentPlayer) {
            return true;
        }
    }

    // Check diagonals
    if (board[0] == currentPlayer &&
        board[4] == currentPlayer &&
        board[8] == currentPlayer) {
        return true;
    }

    if (board[2] == currentPlayer &&
        board[4] == currentPlayer &&
        board[6] == currentPlayer) {
        return true;
    }

    return false;
}

bool AdvancedTicTacToe::checkDraw()
{
    // If any cell is empty, it's not a draw
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            return false;
        }
    }

    return true;
}

void AdvancedTicTacToe::newGame()
{
    // Reset game state
    playerXTurn = true;
    gameEnded = false;

    // Clear the board
    board = QVector<int>(9, 0);

    // Reset all buttons
    for (QPushButton *button : cells) {
        button->setText("");
    }

    // Reset status label
    statusLabel->setText("Player X's turn");
}

void AdvancedTicTacToe::saveGame()
{
    if (!currentUser.isEmpty()) {
        // In a real app, this would save the game state to a database
        QMessageBox::information(this, "Game Saved", "Game saved successfully.");
    } else {
        QMessageBox::warning(this, "Error", "Please log in to save games.");
    }
}

void AdvancedTicTacToe::setDifficulty(const QString &level)
{
    currentDifficulty = level;

    // Visually update selected difficulty button
    easyButton->setStyleSheet("");
    mediumButton->setStyleSheet("");
    hardButton->setStyleSheet("");
    expertButton->setStyleSheet("");

    if (level == "Easy") {
        easyButton->setStyleSheet("background-color: #153055;");
    } else if (level == "Medium") {
        mediumButton->setStyleSheet("background-color: #153055;");
    } else if (level == "Hard") {
        hardButton->setStyleSheet("background-color: #153055;");
    } else if (level == "Expert") {
        expertButton->setStyleSheet("background-color: #153055;");
    }

    // If in the middle of a game against AI, restart
    if (playingAgainstAI && !gameEnded) {
        QMessageBox::information(this, "Difficulty Changed",
                                 "Difficulty set to " + level + ". A new game will start.");
        newGame();
    }
}

void AdvancedTicTacToe::updateGameHistory(const QString &result)
{
    if (currentUser.isEmpty()) return;

    // Add game result to history
    QString gameNumber = QString::number(gameHistory[currentUser].size() + 1);
    QString historyEntry = "Game #" + gameNumber + " " + result;

    gameHistory[currentUser].append(historyEntry);
    gameHistoryList->addItem(historyEntry);

    // Save user data
    saveUserData();
}
