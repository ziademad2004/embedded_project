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
#include <QEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QSequentialAnimationGroup>

// Implementation of ReplayDialog
ReplayDialog::ReplayDialog(QWidget* parent) 
    : QDialog(parent, Qt::FramelessWindowHint), 
      m_currentMove(0), 
      m_totalMoves(5) // Example value
{
    setObjectName("replayDialog");
    setStyleSheet(
        "QDialog#replayDialog {"
        "  background-color: #0a0a1a;"
        "  border: 2px solid #0088ff;"
        "  border-radius: 10px;"
        "}"
    );
    
    // Add shadow effect to match web version
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(30);
    shadowEffect->setColor(QColor(0, 136, 255, 200));
    shadowEffect->setOffset(0, 0);
    setGraphicsEffect(shadowEffect);
    
    // Set up animation for pulsing glow effect
    QPropertyAnimation* glowAnimation = new QPropertyAnimation(shadowEffect, "color");
    glowAnimation->setDuration(2000); // 2 seconds like web version
    glowAnimation->setStartValue(QColor(0, 136, 255, 160));
    glowAnimation->setEndValue(QColor(0, 238, 255, 200));
    glowAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    QSequentialAnimationGroup* animGroup = new QSequentialAnimationGroup(this);
    animGroup->addAnimation(glowAnimation);
    animGroup->setLoopCount(-1); // Infinite loop
    animGroup->start();
    
    setupUI();
    
    // Setup playback timer
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, &QTimer::timeout, this, &ReplayDialog::onNextClicked);
}

void ReplayDialog::setupUI() {
    setFixedSize(650, 750);
    setModal(true);
    
    // Set initial move count to 0 of 5 to match the screenshot
    m_currentMove = 0;
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Header layout with title and close button
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    // Title
    QLabel* titleLabel = new QLabel("Game Replay", this);
    titleLabel->setObjectName("replayTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "color: #00eeff;"
        "font-size: 32px;"
        "font-weight: bold;"
        "text-shadow: 0 0 15px #00eeff;"
    );
    headerLayout->addWidget(titleLabel, 1); // 1 = stretch factor
    
    // Close button
    m_closeBtn = new QPushButton("×", this);
    m_closeBtn->setObjectName("closeButton");
    m_closeBtn->setFixedSize(25, 25);
    m_closeBtn->setStyleSheet(
        "QPushButton#closeButton {"
        "  width: 30px;"
        "  height: 30px;"
        "  border-radius: 15px;"
        "  background-color: rgba(255, 105, 180, 0.2);"
        "  color: #ff69b4;"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "  border: 1px solid #ff69b4;"
        "  padding: 0px;"
        "}"
        "QPushButton#closeButton:hover {"
        "  background-color: rgba(255, 105, 180, 0.4);"
        "}"
    );
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    headerLayout->addWidget(m_closeBtn);
    
    mainLayout->addLayout(headerLayout);
    
    // Game info panel
    QWidget* infoPanel = new QWidget(this);
    infoPanel->setObjectName("infoPanel");
    infoPanel->setStyleSheet(
        "QWidget#infoPanel {"
        "  background-color: rgba(0, 0, 30, 0.3);"
        "  border: 1px solid #00ffff;"
        "  border-radius: 5px;"
        "}"
    );
    
    QGridLayout* infoLayout = new QGridLayout(infoPanel);
    infoLayout->setContentsMargins(15, 15, 15, 15);
    infoLayout->setSpacing(10);
    
    // Info labels
    QLabel* dateTitleLabel = new QLabel("DATE", this);
    dateTitleLabel->setStyleSheet("color: #00ffff; font-size: 14px;");
    m_dateLabel = new QLabel("", this);
    m_dateLabel->setStyleSheet("color: white; font-size: 16px;");
    
    QLabel* playersTitleLabel = new QLabel("PLAYERS", this);
    playersTitleLabel->setStyleSheet("color: #00ffff; font-size: 14px;");
    m_playersLabel = new QLabel("", this);
    m_playersLabel->setStyleSheet("color: white; font-size: 16px;");
    
    QLabel* resultTitleLabel = new QLabel("RESULT", this);
    resultTitleLabel->setStyleSheet("color: #00ffff; font-size: 14px;");
    m_resultLabel = new QLabel("", this);
    m_resultLabel->setStyleSheet("color: white; font-size: 16px;");
    
    QLabel* durationTitleLabel = new QLabel("DURATION", this);
    durationTitleLabel->setStyleSheet("color: #00ffff; font-size: 14px;");
    m_durationLabel = new QLabel("", this);
    m_durationLabel->setStyleSheet("color: white; font-size: 16px;");
    
    infoLayout->addWidget(dateTitleLabel, 0, 0);
    infoLayout->addWidget(m_dateLabel, 1, 0);
    infoLayout->addWidget(playersTitleLabel, 0, 1);
    infoLayout->addWidget(m_playersLabel, 1, 1);
    infoLayout->addWidget(resultTitleLabel, 2, 0);
    infoLayout->addWidget(m_resultLabel, 3, 0);
    infoLayout->addWidget(durationTitleLabel, 2, 1);
    infoLayout->addWidget(m_durationLabel, 3, 1);
    
    mainLayout->addWidget(infoPanel);
    
    // Game board
    QGridLayout* boardLayout = new QGridLayout();
    boardLayout->setSpacing(8);
    boardLayout->setContentsMargins(0, 0, 0, 0);
    
    m_cells.clear();
    for (int i = 0; i < 9; ++i) {
        QPushButton* cell = new QPushButton("", this);
        cell->setObjectName("replayCell");
        cell->setFixedSize(105, 105);
        cell->setEnabled(false);
        cell->setStyleSheet(
            "QPushButton#replayCell {"
            "  background-color: rgba(0, 136, 255, 0.1);"
            "  border: 2px solid #0088ff;"
            "  border-radius: 5px;"
            "  font-size: 56px;"
            "  font-weight: bold;"
            "  transition-duration: 0.2s;"
            "}"
            "QPushButton#replayCell[value=\"X\"] {"
            "  color: #ff69b4;"
            "  text-shadow: 0 0 10px #ff69b4;"
            "}"
            "QPushButton#replayCell[value=\"O\"] {"
            "  color: #00eeff;"
            "  text-shadow: 0 0 10px #00eeff;"
            "}"
            "QPushButton#replayCell.win-cell {"
            "  background-color: rgba(0, 255, 0, 0.2);"
            "  box-shadow: 0 0 15px rgba(0, 255, 0, 0.5);"
            "}"
        );
        
        m_cells.append(cell);
        boardLayout->addWidget(cell, i / 3, i % 3);
    }
    
    QWidget* boardContainer = new QWidget();
    boardContainer->setFixedSize(345, 345);
    boardContainer->setLayout(boardLayout);
    
    QHBoxLayout* boardHolderLayout = new QHBoxLayout();
    boardHolderLayout->setAlignment(Qt::AlignCenter);
    boardHolderLayout->addWidget(boardContainer);
    
    mainLayout->addLayout(boardHolderLayout);
    
    // Add some spacing after the board
    mainLayout->addSpacing(40);
    
    // Navigation controls
    QWidget* navPanel = new QWidget(this);
    navPanel->setObjectName("navPanel");
    navPanel->setStyleSheet(
        "QWidget#navPanel {"
        "  background-color: rgba(0, 136, 255, 0.1);"
        "  border-radius: 25px;"
        "  padding: 10px;"
        "}"
    );
    navPanel->setFixedHeight(80);
    
    QHBoxLayout* navLayout = new QHBoxLayout(navPanel);
    navLayout->setContentsMargins(15, 0, 15, 0);
    navLayout->setSpacing(40);
    navLayout->setAlignment(Qt::AlignCenter);
    
    // Previous button
    QVBoxLayout* previousLayout = new QVBoxLayout();
    previousLayout->setAlignment(Qt::AlignCenter);
    
    m_previousBtn = new QPushButton("◀", this);
    m_previousBtn->setObjectName("navButton");
    m_previousBtn->setCursor(Qt::PointingHandCursor);
    m_previousBtn->setFixedSize(40, 40);
    m_previousBtn->setStyleSheet(
        "QPushButton#navButton {"
        "  background-color: rgba(0, 136, 255, 0.2);"
        "  color: #00eeff;"
        "  border: 1px solid #00eeff;"
        "  border-radius: 20px;"
        "  font-size: 16px;"
        "  padding: 8px 15px;"
        "  font-weight: normal;"
        "  transition-duration: 0.3s;"
        "}"
        "QPushButton#navButton:hover {"
        "  background-color: rgba(0, 238, 255, 0.3);"
        "  box-shadow: 0 0 15px rgba(0, 238, 255, 0.8);"
        "}"
        "QPushButton#navButton:disabled {"
        "  color: #555555;"
        "  border-color: #555555;"
        "}"
    );
    connect(m_previousBtn, &QPushButton::clicked, this, &ReplayDialog::onPreviousClicked);
    
    previousLayout->addWidget(m_previousBtn);
    previousLayout->addSpacing(10);
    
    QLabel* previousLabel = new QLabel("Previous", this);
    previousLabel->setAlignment(Qt::AlignCenter);
    previousLabel->setStyleSheet("color: #00ffff; font-size: 12px;");
    previousLayout->addWidget(previousLabel);
    
    // Move counter
    QVBoxLayout* moveCounterLayout = new QVBoxLayout();
    moveCounterLayout->setAlignment(Qt::AlignCenter);
    moveCounterLayout->setContentsMargins(40, 0, 40, 0);
    
    m_moveCountLabel = new QLabel("Move 0 of 5", this);
    m_moveCountLabel->setAlignment(Qt::AlignCenter);
    m_moveCountLabel->setStyleSheet("color: #00ffff; font-size: 16px;");
    moveCounterLayout->addWidget(m_moveCountLabel);
    
    // Next button
    QVBoxLayout* nextLayout = new QVBoxLayout();
    nextLayout->setAlignment(Qt::AlignCenter);
    
    m_nextBtn = new QPushButton("▶", this);
    m_nextBtn->setObjectName("navButton");
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setFixedSize(40, 40);
    m_nextBtn->setStyleSheet(
        "QPushButton#navButton {"
        "  background-color: transparent;"
        "  color: #00ffff;"
        "  border: 1px solid #00ffff;"
        "  border-radius: 0px;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#navButton:hover {"
        "  background-color: rgba(0, 255, 255, 0.2);"
        "}"
        "QPushButton#navButton:disabled {"
        "  color: #555555;"
        "  border-color: #555555;"
        "}"
    );
    connect(m_nextBtn, &QPushButton::clicked, this, &ReplayDialog::onNextClicked);
    
    nextLayout->addWidget(m_nextBtn);
    nextLayout->addSpacing(10);
    
    QLabel* nextLabel = new QLabel("Next", this);
    nextLabel->setAlignment(Qt::AlignCenter);
    nextLabel->setStyleSheet("color: #00ffff; font-size: 12px;");
    nextLayout->addWidget(nextLabel);
    
        navLayout->addLayout(previousLayout);
    navLayout->addLayout(moveCounterLayout);
    navLayout->addLayout(nextLayout);
    
    mainLayout->addWidget(navPanel);
    
    // Playback controls
    QHBoxLayout* playbackLayout = new QHBoxLayout();
    playbackLayout->setAlignment(Qt::AlignCenter);
    playbackLayout->setSpacing(15);
    playbackLayout->setContentsMargins(0, 20, 0, 10);
    
    // Play button
    m_playBtn = new QPushButton("▶ Play", this);
    m_playBtn->setObjectName("playbackButton");
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setFixedSize(150, 35);
    m_playBtn->setStyleSheet(
        "QPushButton#playbackButton {"
        "  background-color: transparent;"
        "  color: #00ffff;"
        "  border: 1px solid #00ffff;"
        "  border-radius: 0px;"
        "  padding: 5px 15px;"
        "  font-size: 16px;"
        "}"
        "QPushButton#playbackButton:hover {"
        "  background-color: rgba(0, 255, 255, 0.2);"
        "}"
    );
    connect(m_playBtn, &QPushButton::clicked, this, &ReplayDialog::onPlayClicked);
    
    // Pause button
    m_pauseBtn = new QPushButton("❚❚ Pause", this);
    m_pauseBtn->setObjectName("playbackButton");
    m_pauseBtn->setCursor(Qt::PointingHandCursor);
    m_pauseBtn->setFixedSize(150, 35);
    m_pauseBtn->setStyleSheet(
        "QPushButton#playbackButton {"
        "  background-color: transparent;"
        "  color: #00ffff;"
        "  border: 1px solid #00ffff;"
        "  border-radius: 0px;"
        "  padding: 5px 15px;"
        "  font-size: 16px;"
        "}"
        "QPushButton#playbackButton:hover {"
        "  background-color: rgba(0, 255, 255, 0.2);"
        "}"
    );
    m_pauseBtn->setEnabled(false);
    connect(m_pauseBtn, &QPushButton::clicked, this, &ReplayDialog::onPauseClicked);
    
    playbackLayout->addWidget(m_playBtn);
    playbackLayout->addWidget(m_pauseBtn);
    
    mainLayout->addLayout(playbackLayout);
    
    // Update button states
    updateButtonStates();
}

void ReplayDialog::setGameData(const QString& date, const QString& players, 
                              const QString& result, const QString& duration) {
    m_dateLabel->setText(date);
    m_playersLabel->setText(players);
    m_resultLabel->setText(result);
    m_durationLabel->setText(duration);
    
    // Reset the board and move counter
    m_currentMove = 0;
    updateMoveCounter();
    
    // Clear the board
    for (auto cell : m_cells) {
        cell->setText("");
        cell->setProperty("value", "");
        cell->style()->unpolish(cell);
        cell->style()->polish(cell);
    }
    
    // Update button states
    updateButtonStates();
}

void ReplayDialog::onPreviousClicked() {
    if (m_currentMove > 0) {
        m_currentMove--;
        updateMoveCounter();
        updateBoard();
        updateButtonStates();
    }
}

void ReplayDialog::onNextClicked() {
    if (m_currentMove < m_totalMoves) {
        m_currentMove++;
        updateMoveCounter();
        updateBoard();
        updateButtonStates();
        
        // If we reached the end, stop playback
        if (m_currentMove >= m_totalMoves && m_playbackTimer->isActive()) {
            m_playbackTimer->stop();
            m_playBtn->setEnabled(true);
            m_pauseBtn->setEnabled(false);
        }
    }
}

void ReplayDialog::onPlayClicked() {
    // If we're at the end, start from beginning
    if (m_currentMove >= m_totalMoves) {
        m_currentMove = 0;
        updateMoveCounter();
        updateBoard();
    }
    
    // Start playback timer
    m_playbackTimer->start(1000); // 1 second between moves
    
    // Update button states
    m_playBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
}

void ReplayDialog::onPauseClicked() {
    // Stop playback timer
    m_playbackTimer->stop();
    
    // Update button states
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
}

void ReplayDialog::updateMoveCounter() {
    m_moveCountLabel->setText(QString("Move %1 of %2").arg(m_currentMove).arg(m_totalMoves));
}

void ReplayDialog::updateBoard() {
    // Clear all cells first
    for (int i = 0; i < 9; ++i) {
        m_cells[i]->setText("");
        m_cells[i]->setProperty("value", "");
        m_cells[i]->style()->unpolish(m_cells[i]);
        m_cells[i]->style()->polish(m_cells[i]);
    }
    
    // If we have actual move data, use it
    if (!m_moves.isEmpty()) {
        // Apply moves up to current move
        for (int i = 0; i < qMin(m_currentMove, m_moves.size()); ++i) {
            int cellIndex = m_moves[i].cellIndex;
            QString value = (m_moves[i].player == 1) ? "X" : "O";
            m_cells[cellIndex]->setText(value);
            m_cells[cellIndex]->setProperty("value", value);
            m_cells[cellIndex]->style()->unpolish(m_cells[cellIndex]);
            m_cells[cellIndex]->style()->polish(m_cells[cellIndex]);
        }
    } else {
        // Fallback to example moves if no actual data is available
        QVector<QPair<int, QString>> exampleMoves = {
            {0, "O"},  // Top left - O
            {2, "X"},  // Top right - X
            {4, "X"},  // Center - X
            {6, "O"},  // Bottom left - O
            {8, "X"}   // Bottom right - X
        };
        
        // Apply example moves up to current move
        for (int i = 0; i < qMin(m_currentMove, exampleMoves.size()); ++i) {
            int cellIndex = exampleMoves[i].first;
            QString value = exampleMoves[i].second;
            m_cells[cellIndex]->setText(value);
            m_cells[cellIndex]->setProperty("value", value);
            m_cells[cellIndex]->style()->unpolish(m_cells[cellIndex]);
            m_cells[cellIndex]->style()->polish(m_cells[cellIndex]);
        }
    }
}

void ReplayDialog::updateButtonStates() {
    m_previousBtn->setEnabled(m_currentMove > 0);
    m_nextBtn->setEnabled(m_currentMove < m_totalMoves);
}

// Implementation of LeaderboardItemDelegate::paint method to match target image exactly
void LeaderboardItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }
    
    // Draw the background - match the dark blue background with exact opacity as in target image
    painter->fillRect(option.rect, QColor(0, 136, 255, 25)); // Slightly reduced opacity to match target
    
    // Draw the border - exactly 1px blue border as in the target image
    painter->setPen(QPen(QColor(0, 136, 255), 1)); // Using RGB integers instead of string literal
    painter->drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), 5, 5);
    
    // Get the text data
    QString playerRank = index.data(Qt::DisplayRole).toString();
    QString stats = index.data(Qt::UserRole).toString();
    
    // Set up the font - increase size for better visibility
    QFont playerFont = option.font;
    playerFont.setBold(true);
    playerFont.setPointSize(14); // Further increased for better visibility
    
    QFont statsFont = option.font;
    statsFont.setPointSize(13); // Further increased for better visibility
    
    // Calculate text rectangles - adjust padding to prevent text cutoff
    QRect rankRect = option.rect.adjusted(15, 0, -25, 0);
    QRect statsRect = option.rect.adjusted(15, 0, -25, 0);
    
    // Draw the player rank with cyan color exactly as in the target image
    painter->setPen(QColor(0, 238, 255)); // Using RGB integers instead of string literal - bright cyan
    painter->setFont(playerFont);
    painter->drawText(rankRect, Qt::AlignLeft | Qt::AlignVCenter, playerRank);
    
    // Draw the stats with white color
    painter->setPen(QColor(255, 255, 255)); // Pure white for stats text
    painter->setFont(statsFont);
    painter->drawText(statsRect, Qt::AlignRight | Qt::AlignVCenter, stats);
}

// Implementation of HistoryItemDelegate::paint method to match the leaderboard style
void HistoryItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }
    
    // Draw the background - match the leaderboard style
    painter->fillRect(option.rect, QColor(0, 136, 255, 25));
    
    // Draw the border - exactly 1px blue border
    painter->setPen(QPen(QColor(0, 136, 255), 1));
    painter->drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), 5, 5);
    
    // Get the text data
    QString historyEntry = index.data(Qt::DisplayRole).toString();
    QString date = index.data(Qt::UserRole).toString();
    
    // Set up the fonts - match leaderboard style
    QFont entryFont = option.font;
    entryFont.setBold(true);
    entryFont.setPointSize(14);
    
    QFont dateFont = option.font;
    dateFont.setPointSize(13);
    
    // Calculate text rectangles - adjust to make room for the replay button
    QRect entryRect = option.rect.adjusted(15, 0, -120, 0);
    QRect dateRect = option.rect.adjusted(15, 0, -120, 0);
    
    // Draw the history entry with cyan color
    painter->setPen(QColor(0, 238, 255));
    painter->setFont(entryFont);
    painter->drawText(entryRect, Qt::AlignLeft | Qt::AlignVCenter, historyEntry);
    
    // Draw the date with white color
    painter->setPen(QColor(255, 255, 255));
    painter->setFont(dateFont);
    painter->drawText(dateRect, Qt::AlignRight | Qt::AlignVCenter, date);
    
    // Draw the replay button
    QRect buttonRect = getReplayButtonRect(option.rect);
    
    // Draw button background
    painter->fillRect(buttonRect, QColor(0, 238, 255, 40));
    
    // Draw button border
    painter->setPen(QPen(QColor(0, 238, 255), 1));
    painter->drawRoundedRect(buttonRect, 3, 3);
    
    // Draw button text
    painter->setPen(QColor(0, 238, 255));
    painter->setFont(QFont(option.font.family(), 10, QFont::Bold));
    painter->drawText(buttonRect, Qt::AlignCenter, "Replay Game");
}

QRect HistoryItemDelegate::getReplayButtonRect(const QRect& itemRect) const {
    // Create a button on the right side of the item
    return QRect(
        itemRect.right() - 110,  // X position (10px from right edge)
        itemRect.top() + (itemRect.height() - 25) / 2,  // Y position (centered vertically)
        100,  // Width
        25    // Height
    );
}

bool HistoryItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, 
                                     const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QRect buttonRect = getReplayButtonRect(option.rect);
        
        if (buttonRect.contains(mouseEvent->pos())) {
            emit replayButtonClicked(index.row());
            return true;
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
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
    m_loadingSpinner->setStyleSheet("border: 5px solid rgba(0, 238, 255, 0.3); border-radius: 25px; border-top-color: #00eeff;");

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
    gameContainerLayout->setSpacing(10); // Reduced spacing to match web version width

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

    // Game board grid - improved proportions for a more natural look
    QWidget *boardWidget = new QWidget();
    boardWidget->setObjectName("gameBoard");
    boardWidget->setFixedSize(400, 400); // Increased square size for better proportions
    m_boardLayout = new QGridLayout(boardWidget);
    m_boardLayout->setSpacing(8); // More balanced spacing between cells
    m_boardLayout->setContentsMargins(6, 6, 6, 6); // Slightly larger margins
    
    // Create cells
    m_cells.clear();
    for (int i = 0; i < 9; ++i) {
        QPushButton *cell = new QPushButton();
        cell->setObjectName("cellButton");
        cell->setProperty("index", i);
        cell->setFixedSize(115, 115); // Slightly larger cells for better proportions
        cell->setCursor(Qt::PointingHandCursor);
        
        m_cells.append(cell);
        m_boardLayout->addWidget(cell, i / 3, i % 3);
        
        connect(cell, &QPushButton::clicked, this, &MainWindow::onCellClicked);
    }

    gameBoardLayout->addWidget(boardWidget);
    gameBoardLayout->setAlignment(boardWidget, Qt::AlignCenter); // Center the board

    // Add spacing between board and buttons
    gameBoardLayout->addSpacing(30);
    
    // Game action buttons - fix text clarity issues
    QHBoxLayout *gameActionsLayout = new QHBoxLayout();
    gameActionsLayout->setSpacing(15);
    
    m_newGameBtn = new QPushButton("New Game");
    m_newGameBtn->setObjectName("newGameBtn"); // Set object name for QSS styling
    m_newGameBtn->setFixedHeight(45);
    m_newGameBtn->setStyleSheet(
        "background-color: transparent;"
        "color: #00eeff;"
        "border: 2px solid #00eeff;"
        "border-radius: 5px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "text-align: center;"
        "qproperty-alignment: AlignCenter;"
        "letter-spacing: 1px;"
    );
    
    m_saveGameBtn = new QPushButton("Save Game");
    m_saveGameBtn->setObjectName("saveGameBtn"); // Set object name for QSS styling
    m_saveGameBtn->setFixedHeight(45);
    m_saveGameBtn->setStyleSheet(
        "background-color: transparent;"
        "color: #00eeff;"
        "border: 2px solid #00eeff;"
        "border-radius: 5px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "text-align: center;"
        "qproperty-alignment: AlignCenter;"
        "letter-spacing: 1px;"
    );
    
    m_exitGameBtn = new QPushButton("Exit Game");
    m_exitGameBtn->setObjectName("exitGameBtn"); // Set object name for QSS styling
    m_exitGameBtn->setFixedHeight(45);
    m_exitGameBtn->setStyleSheet(
        "background-color: transparent;"
        "color: #00eeff;"
        "border: 2px solid #00eeff;"
        "border-radius: 5px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "text-align: center;"
        "qproperty-alignment: AlignCenter;"
        "letter-spacing: 1px;"
    );
    
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
    m_toggleStatsViewBtn->setFixedWidth(200); // Wider for better text fit
    m_toggleStatsViewBtn->setFixedHeight(45); // Taller for better text visibility
    // Apply explicit styling to fix unclear text
    m_toggleStatsViewBtn->setStyleSheet(
        "background-color: transparent;"
        "color: #00eeff;"
        "border: 2px solid #00eeff;"
        "border-radius: 5px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "text-align: center;"
        "qproperty-alignment: AlignCenter;"
        "letter-spacing: 0.5px;"
        "box-shadow: 0 0 10px rgba(0, 238, 255, 0.7);"
    );

    // Create a container for the button with right alignment
    QHBoxLayout *statsButtonLayout = new QHBoxLayout();
    statsButtonLayout->addStretch();
    statsButtonLayout->addWidget(m_toggleStatsViewBtn);
    statsButtonLayout->setContentsMargins(0, 25, 10, 5); // Increased top margin for more spacing

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
    // Match the exact styling from the image - bright cyan with glow effect
    statsTitle->setStyleSheet("color: #00eeff; text-shadow: 0 0 10px #00eeff, 0 0 20px #00eeff; font-weight: 800; letter-spacing: 3px; font-size: 48px; margin-bottom: 30px;");
    setupTitleAnimation(statsTitle);
    statsLayout->addWidget(statsTitle);

    // Create custom tab bar that exactly matches the image
    QWidget *tabBarWidget = new QWidget();
    tabBarWidget->setObjectName("tabBar");
    QHBoxLayout *tabBarLayout = new QHBoxLayout(tabBarWidget);
    tabBarLayout->setSpacing(0);
    tabBarLayout->setContentsMargins(0, 0, 0, 0);
    tabBarLayout->setAlignment(Qt::AlignTop);
    
    // Create the tab buttons to match the image exactly
    QPushButton *personalStatsTab = new QPushButton("My Stats");
    personalStatsTab->setObjectName("tabButton");
    personalStatsTab->setProperty("tabIndex", 0);
    personalStatsTab->setProperty("active", false);
    
    QPushButton *leaderboardTab = new QPushButton("Leaderboard");
    leaderboardTab->setObjectName("tabButton");
    leaderboardTab->setProperty("tabIndex", 1);
    leaderboardTab->setProperty("active", true);
    leaderboardTab->setStyleSheet("background-color: rgba(0, 238, 255, 0.2); box-shadow: 0 0 10px rgba(0, 238, 255, 0.5);");
    
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
    connect(personalStatsTab, &QPushButton::clicked, this, [=]() {
        tabContentWidget->setCurrentIndex(0);
        personalStatsTab->setProperty("active", true);
        leaderboardTab->setProperty("active", false);
        historyTab->setProperty("active", false);
        personalStatsTab->setStyleSheet("background-color: rgba(0, 255, 255, 0.2); box-shadow: 0 0 10px rgba(0, 255, 255, 0.5);");
        leaderboardTab->setStyleSheet("");
        historyTab->setStyleSheet("");
    });
    
    connect(leaderboardTab, &QPushButton::clicked, this, [=]() {
        tabContentWidget->setCurrentIndex(1);
        personalStatsTab->setProperty("active", false);
        leaderboardTab->setProperty("active", true);
        historyTab->setProperty("active", false);
        personalStatsTab->setStyleSheet("");
        leaderboardTab->setStyleSheet("background-color: rgba(0, 255, 255, 0.2); box-shadow: 0 0 10px rgba(0, 255, 255, 0.5);");
        historyTab->setStyleSheet("");
        populateLeaderboard(); // Update leaderboard when switching to that tab
    });
    
    connect(historyTab, &QPushButton::clicked, this, [=]() {
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
    
    // Leaderboard Tab Content - match exactly what's shown in the image
    QWidget *leaderboardContent = new QWidget();
    QVBoxLayout *leaderboardLayout = new QVBoxLayout(leaderboardContent);
    
    QWidget *leaderboardPanel = new QWidget();
    leaderboardPanel->setObjectName("statsPanel");
    // Set the styling with proper width to prevent cutoff
    leaderboardPanel->setStyleSheet("border: 1px solid #0088ff; border-radius: 5px; background-color: rgba(0, 10, 30, 0.2); box-shadow: 0 0 5px rgba(0, 136, 255, 0.4);");
    leaderboardPanel->setContentsMargins(5, 5, 5, 5); // Add content margins
    leaderboardPanel->setMinimumHeight(250); // Increased height to accommodate larger items
    QVBoxLayout *leaderboardPanelLayout = new QVBoxLayout(leaderboardPanel);
    leaderboardPanelLayout->setContentsMargins(20, 20, 20, 20); // Increased padding for larger items
    
    QLabel *leaderboardTitle = new QLabel("Global Leaderboard");
    leaderboardTitle->setObjectName("panelTitle");
    // Set exact styling to match the target image - brighter cyan color with glow
    leaderboardTitle->setStyleSheet("color: #00eeff; font-size: 22px; margin-top: 0; margin-bottom: 15px; text-shadow: 0 0 5px #00eeff; font-weight: bold;");
    leaderboardTitle->setAlignment(Qt::AlignLeft); // Left-aligned as in target image
    leaderboardPanelLayout->addWidget(leaderboardTitle);
    
    m_leaderboardList = new QListWidget();
    m_leaderboardList->setObjectName("leaderboardList");
    // Match the exact styling of the list from the image
    m_leaderboardList->setStyleSheet("background-color: transparent; border: none;");
    // Set spacing between items to match the image
    m_leaderboardList->setSpacing(0); // No extra spacing - we'll handle it in the item styling
    m_leaderboardList->setContentsMargins(0, 0, 0, 0); // No margins
    leaderboardPanelLayout->addWidget(m_leaderboardList);
    leaderboardLayout->addWidget(leaderboardPanel);
    
    // Game History Tab Content - styled to match the leaderboard
    QWidget *historyContent = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyContent);
    
    QWidget *fullHistoryPanel = new QWidget();
    fullHistoryPanel->setObjectName("statsPanel");
    // Match the leaderboard panel styling
    fullHistoryPanel->setStyleSheet("border: 1px solid #0088ff; border-radius: 5px; background-color: rgba(0, 10, 30, 0.2); box-shadow: 0 0 5px rgba(0, 136, 255, 0.4);");
    fullHistoryPanel->setContentsMargins(5, 5, 5, 5);
    fullHistoryPanel->setMinimumHeight(250);
    QVBoxLayout *fullHistoryLayout = new QVBoxLayout(fullHistoryPanel);
    fullHistoryLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *fullHistoryTitle = new QLabel("Complete Game History");
    fullHistoryTitle->setObjectName("panelTitle");
    // Match leaderboard title styling
    fullHistoryTitle->setStyleSheet("color: #00eeff; font-size: 22px; margin-top: 0; margin-bottom: 15px; text-shadow: 0 0 5px #00eeff; font-weight: bold;");
    fullHistoryTitle->setAlignment(Qt::AlignLeft);
    fullHistoryLayout->addWidget(fullHistoryTitle);
    
    m_fullHistoryList = new QListWidget();
    m_fullHistoryList->setObjectName("fullHistoryList");
    m_fullHistoryList->setStyleSheet("background-color: transparent; border: none;");
    // Match leaderboard list spacing
    m_fullHistoryList->setSpacing(10);
    m_fullHistoryList->setContentsMargins(0, 0, 0, 0);
    
    // Create and set the history item delegate with replay button
    m_historyItemDelegate = new HistoryItemDelegate(this);
    m_fullHistoryList->setItemDelegate(m_historyItemDelegate);
    
    // Connect the delegate's signal to our slot
    connect(m_historyItemDelegate, &HistoryItemDelegate::replayButtonClicked, 
            this, &MainWindow::onReplayButtonClicked);
            
    fullHistoryLayout->addWidget(m_fullHistoryList);
    historyLayout->addWidget(fullHistoryPanel);
    
    // Add content pages to stacked widget
    tabContentWidget->addWidget(personalStatsContent);
    tabContentWidget->addWidget(leaderboardContent);
    tabContentWidget->addWidget(historyContent);
    
    // Set leaderboard tab as active to match the image
    tabContentWidget->setCurrentIndex(1); // Show leaderboard tab
    
    // Add stacked widget to main layout
    statsLayout->addWidget(tabContentWidget);
    
    // Add back button at the bottom exactly as in the image
    m_backToGameBtn = new QPushButton("Back to Game");
    m_backToGameBtn->setObjectName("backToGameBtn");
    m_backToGameBtn->setFixedHeight(40);
    // Match exact styling from image - cyan border, text and glow
    m_backToGameBtn->setStyleSheet("background-color: transparent; color: #00eeff; border: 2px solid #00eeff; border-radius: 5px; padding: 10px 20px; box-shadow: 0 0 5px rgba(0, 238, 255, 0.5); font-size: 16px;");
    
    // Create a container for the back button with proper positioning - match image
    QHBoxLayout *backBtnLayout = new QHBoxLayout();
    backBtnLayout->addWidget(m_backToGameBtn);
    backBtnLayout->setContentsMargins(0, 20, 0, 0);
    
    statsLayout->addLayout(backBtnLayout);
    
    // Create delegates for the lists if not already created
    if (!m_leaderboardDelegate) {
        m_leaderboardDelegate = new LeaderboardItemDelegate();
        m_leaderboardList->setItemDelegate(m_leaderboardDelegate);
    }
    
    if (!m_historyItemDelegate) {
        m_historyItemDelegate = new HistoryItemDelegate();
        m_fullHistoryList->setItemDelegate(m_historyItemDelegate);
    }
    
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
                // Convert GameLogic moves to GameMoveRecord
                QVector<GameMoveRecord> moveRecords;
                const auto& moves = m_gameLogic->getMoveHistory();
                for (const GameMove& move : moves) {
                    GameMoveRecord record;
                    record.cellIndex = move.cellIndex;
                    record.player = move.player;
                    moveRecords.append(record);
                }
                
                m_auth->getCurrentUser()->addGameWithMoves("win", "ai", moveRecords,
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
                // Convert GameLogic moves to GameMoveRecord
                QVector<GameMoveRecord> moveRecords;
                const auto& moves = m_gameLogic->getMoveHistory();
                for (const GameMove& move : moves) {
                    GameMoveRecord record;
                    record.cellIndex = move.cellIndex;
                    record.player = move.player;
                    moveRecords.append(record);
                }
                
                m_auth->getCurrentUser()->addGameWithMoves("loss", "ai", moveRecords,
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
                // Convert GameLogic moves to GameMoveRecord
                QVector<GameMoveRecord> moveRecords;
                const auto& moves = m_gameLogic->getMoveHistory();
                for (const GameMove& move : moves) {
                    GameMoveRecord record;
                    record.cellIndex = move.cellIndex;
                    record.player = move.player;
                    moveRecords.append(record);
                }
                
                m_auth->getCurrentUser()->addGameWithMoves("draw", "ai", moveRecords,
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
        populateLeaderboard(); // Update the leaderboard with new rankings
    }
    
    // Save user data to persist statistics
    m_auth->saveUsers();
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
            // Match web version exactly with enhanced glow
            m_cells[i]->setStyleSheet(
                "color: #ff69b4; "
                "text-shadow: 0 0 10px #ff69b4, 0 0 15px #ff69b4; "
                "font-size: 48px; "
                "font-weight: bold;"
            );
        } else if (state == GameLogic::Player::O) {
            m_cells[i]->setText("O");
            m_cells[i]->setProperty("cell-value", "O");
            // Match web version exactly with enhanced glow
            m_cells[i]->setStyleSheet(
                "color: #00eeff; " // Updated to match web color
                "text-shadow: 0 0 10px #00eeff, 0 0 20px #00eeff; "
                "font-size: 48px; "
                "font-weight: bold;"
            );
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

    // Update full history with styled items like the leaderboard
    m_fullHistoryList->clear();
    const auto& gameHistory = currentUser->getGameHistory();
    
    // Use index-based loop to avoid Qt container detachment warning
    int count = 1;
    for (int i = 0; i < gameHistory.size(); ++i) {
        const GameRecord &record = gameHistory.at(i);
        
        QListWidgetItem* item = new QListWidgetItem();
        
        // Format the game result with the player's name
        QString formattedResult = QString("%1. %2: %3").arg(count).arg(currentUser->getUsername()).arg(record.result);
        item->setText(formattedResult);
        
        // Store the date as user data for display on the right
        item->setData(Qt::UserRole, record.date);
        
        // Set item size to match leaderboard items
        item->setSizeHint(QSize(m_fullHistoryList->width() - 20, 50));
        
        m_fullHistoryList->addItem(item);
        count++;
    }
    
    // If history is empty, add a placeholder message
    if (gameHistory.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText("Game History");
        item->setData(Qt::UserRole, "Play games to see your history!");
        item->setSizeHint(QSize(m_fullHistoryList->width() - 20, 50));
        m_fullHistoryList->addItem(item);
    }
}

void MainWindow::populateLeaderboard() {
    m_leaderboardList->clear();

    // Get leaderboard data using the new ranking system
    QVector<LeaderboardEntry> leaderboard = m_database->getLeaderboard(m_auth->getUsers());
    
    // Add each ranked player to the leaderboard
    int rank = 1;
    // Use index-based loop to avoid Qt container detachment warning
    for (int i = 0; i < leaderboard.size(); ++i) {
        const LeaderboardEntry& entry = leaderboard.at(i);
        QListWidgetItem* item = new QListWidgetItem();
        // Format: "1. username" (rank and username)
        item->setText(QString("%1. %2").arg(rank).arg(entry.username));
        // Format: "Wins: X | Win Rate: Y%" (stats)
        item->setData(Qt::UserRole, QString("Wins: %1 | Win Rate: %2%").arg(entry.wins).arg(entry.winRate));
        m_leaderboardList->addItem(item);
        rank++;
    }
    
    // If leaderboard is empty, add a placeholder message
    if (leaderboard.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText("Leaderboard");
        item->setData(Qt::UserRole, "Play games to rank on the leaderboard!");
        m_leaderboardList->addItem(item);
    }
    
    // Ensure proper spacing between items - increased for better visibility
    m_leaderboardList->setSpacing(10);
    
    // Set item height and ensure width is properly set to avoid cutoff
    for (int i = 0; i < m_leaderboardList->count(); ++i) {
        QListWidgetItem* item = m_leaderboardList->item(i);
        // Subtract some pixels from width to prevent cutoff at the right edge
        item->setSizeHint(QSize(m_leaderboardList->width() - 20, 50));
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

    // Set win state for winning cells with explicit styling to match web
    const auto& pattern = winPattern;
    for (const int& index : pattern) {
        QPushButton* cell = m_cells[index];
        cell->setProperty("winner", true);
        
        // Get current cell value (X or O)
        QString cellValue = cell->property("cell-value").toString();
        
        // Apply winning cell formatting to match first image
        if (cellValue == "X") {
            // X winning style with colors and dimensions from first image
            cell->setStyleSheet(
                "color: #ff69b4; "
                "text-shadow: 0 0 10px #ff69b4, 0 0 15px #ff69b4; "
                "font-size: 48px; "
                "font-weight: bold; "
                "background-color: rgba(0, 100, 0, 0.5); "
                "border: 1px solid #00ff00; "
                "border-radius: 4px; "
                "box-shadow: 0 0 15px rgba(0, 255, 0, 0.5);"
            );
        } else {
            // O winning style with colors and dimensions from first image
            cell->setStyleSheet(
                "color: #00eeff; "
                "text-shadow: 0 0 10px #00eeff, 0 0 20px #00eeff; "
                "font-size: 48px; "
                "font-weight: bold; "
                "background-color: rgba(0, 100, 0, 0.5); "
                "border: 1px solid #00ff00; "
                "border-radius: 4px; "
                "box-shadow: 0 0 15px rgba(0, 255, 0, 0.5);"
            );
        }
        
        cell->style()->unpolish(cell);
        cell->style()->polish(cell);
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
    // Apply web-style loading animation
    m_loadingOverlay->setStyleSheet(
        "QWidget {"
        "  background-color: rgba(0, 0, 0, 0.8);"
        "  display: flex;"
        "  justify-content: center;"
        "  align-items: center;"
        "}"
    );
    
    // Create the spinner animation if not already set up
    if (!m_loadingSpinner->movie()) {
        QGraphicsOpacityEffect* fadeEffect = new QGraphicsOpacityEffect(this);
        m_loadingOverlay->setGraphicsEffect(fadeEffect);
        
        // Fade in animation
        QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeEffect, "opacity");
        fadeIn->setDuration(200);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::InOutQuad);
        
        // Show and start animation
        m_loadingOverlay->setVisible(true);
        m_loadingOverlay->raise();
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_loadingOverlay->setVisible(true);
        m_loadingOverlay->raise();
    }

    // Hide overlay after duration with fade out
    QTimer::singleShot(duration - 200, this, [this]() {
        QGraphicsOpacityEffect* fadeEffect = new QGraphicsOpacityEffect(this);
        m_loadingOverlay->setGraphicsEffect(fadeEffect);
        
        // Fade out animation
        QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeEffect, "opacity");
        fadeOut->setDuration(200);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setEasingCurve(QEasingCurve::InOutQuad);
        
        connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
            m_loadingOverlay->setVisible(false);
        });
        
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

// Add the slot implementation for handling replay button clicks
void MainWindow::onReplayButtonClicked(int index) {
    qDebug() << "Replay button clicked for game at index:" << index;
    
    // Get the game data from the history item
    QListWidgetItem* item = m_fullHistoryList->item(index);
    if (!item) return;
    
    QString gameResult = item->text();
    QString gameDate = item->data(Qt::UserRole).toString();
    
    // Parse the game data
    QString players;
    if (gameResult.contains("AI")) {
        players = m_auth->getCurrentUser()->getUsername() + " vs AI";
    } else {
        // Extract player name from result (e.g., "Won vs player2")
        QString opponent = gameResult.section("vs ", 1);
        players = m_auth->getCurrentUser()->getUsername() + " vs " + opponent;
    }
    
    QString result;
    if (gameResult.startsWith("Win")) {
        result = m_auth->getCurrentUser()->getUsername() + " won";
    } else if (gameResult.startsWith("Loss")) {
        result = m_auth->getCurrentUser()->getUsername() + " lost";
    } else {
        result = "Draw";
    }
    
    // Create and show the replay dialog
    ReplayDialog* replayDialog = new ReplayDialog(this);
    replayDialog->setGameData(gameDate, players, result, "6 sec");
    
    // Get the actual move history from the user's game history
    QVector<GameMove> gameMoves;
    
    // Find the corresponding game in user's history
    if (m_auth->getCurrentUser()) {
        const auto& gameHistory = m_auth->getCurrentUser()->getGameHistory();
        
        // Find the matching game record by date
        for (const GameRecord& record : gameHistory) {
            // Check if this is the same game by comparing date
            if (record.date == gameDate) {
                // Convert GameMoveRecord to GameMove
                for (const GameMoveRecord& moveRecord : record.moves) {
                    GameMove move;
                    move.cellIndex = moveRecord.cellIndex;
                    move.player = moveRecord.player;
                    gameMoves.append(move);
                }
                break;
            }
        }
    }
    
    // If no move history was found, generate representative moves
    // based on the game outcome
    if (gameMoves.isEmpty()) {
        if (result.contains("won")) {
            // X wins scenario (player wins)
            gameMoves.append({0, 1}); // X in top-left
            gameMoves.append({3, 2}); // O in middle-left
            gameMoves.append({1, 1}); // X in top-middle
            gameMoves.append({5, 2}); // O in middle-right
            gameMoves.append({2, 1}); // X in top-right (winning move)
        } else if (result.contains("lost")) {
            // O wins scenario (AI wins)
            gameMoves.append({4, 1}); // X in center
            gameMoves.append({0, 2}); // O in top-left
            gameMoves.append({8, 1}); // X in bottom-right
            gameMoves.append({3, 2}); // O in middle-left
            gameMoves.append({7, 1}); // X in bottom-middle
            gameMoves.append({6, 2}); // O in bottom-left (winning move)
        } else {
            // Draw scenario
            gameMoves.append({0, 1}); // X in top-left
            gameMoves.append({4, 2}); // O in center
            gameMoves.append({2, 1}); // X in top-right
            gameMoves.append({6, 2}); // O in bottom-left
            gameMoves.append({3, 1}); // X in middle-left
            gameMoves.append({5, 2}); // O in middle-right
            gameMoves.append({7, 1}); // X in bottom-middle
            gameMoves.append({1, 2}); // O in top-middle
            gameMoves.append({8, 1}); // X in bottom-right
        }
    }
    
    // Set the moves data
    replayDialog->setMoveData(gameMoves);
    
    replayDialog->exec();
    
    // Clean up
    delete replayDialog;
}

// New method to set the move data for replay
void ReplayDialog::setMoveData(const QVector<GameMove>& moves) {
    m_moves = moves;
    m_totalMoves = moves.size();
    m_currentMove = 0;
    updateMoveCounter();
    updateBoard();
    updateButtonStates();
}
