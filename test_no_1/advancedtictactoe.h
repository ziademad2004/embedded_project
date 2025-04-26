#ifndef ADVANCEDTICTACTOE_H
#define ADVANCEDTICTACTOE_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVector>
#include <QListWidget>
#include <QStackedWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>

class AdvancedTicTacToe : public QMainWindow
{
    Q_OBJECT

public:
    AdvancedTicTacToe(QWidget *parent = nullptr);
    ~AdvancedTicTacToe();

private slots:
    void cellClicked();
    void newGame();
    void saveGame();
    void login();
    void registerUser();
    void setDifficulty(const QString &level);
    void makeBotMove();

private:
    void setupUI();
    void setupStyleSheet();
    void applyNeonEffect(QWidget *widget, QColor color = QColor(0, 180, 255));
    void checkGameStatus();
    bool checkWin();
    bool checkDraw();
    void updateGameHistory(const QString &result);
    void loadUserData();
    void saveUserData();
    QPoint getBestMove();
    QPoint getEasyMove();
    QPoint getMediumMove();
    QPoint getHardMove();
    QPoint getExpertMove();

    // Main widgets
    QWidget *centralWidget;
    QStackedWidget *mainStack;
    QWidget *authPage;
    QWidget *gamePage;

    // Authentication widgets
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;

    // Game board widgets
    QVector<QPushButton*> cells;
    QPushButton *newGameButton;
    QPushButton *saveGameButton;
    QLabel *statusLabel;
    QListWidget *gameHistoryList;

    // Difficulty buttons
    QPushButton *easyButton;
    QPushButton *mediumButton;
    QPushButton *hardButton;
    QPushButton *expertButton;
    QString currentDifficulty;

    // Game state
    bool playerXTurn; // true for X, false for O
    bool gameEnded;
    QVector<int> board; // 0 - Empty, 1 - X, 2 - O
    bool playingAgainstAI;
    bool isSinglePlayer;

    // User data
    QString currentUser;
    QMap<QString, QString> users; // username -> password
    QMap<QString, QStringList> gameHistory; // username -> history
};

#endif // ADVANCEDTICTACTOE_H
