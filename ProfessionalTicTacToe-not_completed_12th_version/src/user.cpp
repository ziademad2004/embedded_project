//this is user driver
#include "../include/user.h"
#include "../include/authentication.h"
#include <QDateTime>

User::User()
    : m_totalGames(0), m_wins(0), m_losses(0), m_draws(0),
    m_vsAI(0), m_vsPlayers(0), m_winRate(0), m_bestStreak(0), m_currentStreak(0)
{
}

User::User(const QString &username, const QString &hashedPassword)
    : m_username(username), m_hashedPassword(hashedPassword),
    m_totalGames(0), m_wins(0), m_losses(0), m_draws(0),
    m_vsAI(0), m_vsPlayers(0), m_winRate(0), m_bestStreak(0), m_currentStreak(0)
{
}

QString User::getUsername() const {
    return m_username;
}

bool User::checkPassword(const QString &password) const {
    return Authentication::verifyPassword(password, m_hashedPassword);
}

QString User::getHashedPassword() const {
    return m_hashedPassword;
}

int User::getTotalGames() const {
    return m_totalGames;
}

int User::getWins() const {
    return m_wins;
}

int User::getLosses() const {
    return m_losses;
}

int User::getDraws() const {
    return m_draws;
}

int User::getVsAI() const {
    return m_vsAI;
}

int User::getVsPlayers() const {
    return m_vsPlayers;
}

int User::getWinRate() const {
    return m_winRate;
}

int User::getBestStreak() const {
    return m_bestStreak;
}

QVector<GameRecord> User::getGameHistory() const {
    return m_gameHistory;
}

void User::addGame(const QString &result, const QString &opponent, const QString &difficulty) {
    m_totalGames++;
    if (result == "win") {
        m_wins++;
        m_currentStreak++;
        if (m_currentStreak > m_bestStreak) {
            m_bestStreak = m_currentStreak;
        }
    } else if (result == "loss") {
        m_losses++;
        m_currentStreak = 0;
    } else if (result == "draw") {
        m_draws++;
        m_currentStreak = 0;
    }

    if (opponent == "ai") {
        m_vsAI++;
    } else {
        m_vsPlayers++;
    }

    if (m_totalGames > 0) {
        m_winRate = static_cast<int>((static_cast<double>(m_wins) / m_totalGames) * 100);
    }

    GameRecord record;
    record.date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (opponent == "ai") {
        record.result = QString("%1 vs AI (%2)")
        .arg(result == "win" ? "Win" : (result == "loss" ? "Loss" : "Draw"))
            .arg(difficulty);
    } else {
        record.result = QString("%1 vs %2")
        .arg(result == "win" ? "Win" : (result == "loss" ? "Loss" : "Draw"))
            .arg(opponent);
    }

    m_gameHistory.prepend(record);
    // Keep only the last 20 games
    while (m_gameHistory.size() > 20) {
        m_gameHistory.removeLast();
    }
}

void User::addGameWithDate(const QString &result, const QString &opponent, const QString &date, const QString &difficulty) {
    m_totalGames++;
    if (result == "win") {
        m_wins++;
        m_currentStreak++;
        if (m_currentStreak > m_bestStreak) {
            m_bestStreak = m_currentStreak;
        }
    } else if (result == "loss") {
        m_losses++;
        m_currentStreak = 0;
    } else if (result == "draw") {
        m_draws++;
        m_currentStreak = 0;
    }

    if (opponent == "ai") {
        m_vsAI++;
    } else {
        m_vsPlayers++;
    }

    if (m_totalGames > 0) {
        m_winRate = static_cast<int>((static_cast<double>(m_wins) / m_totalGames) * 100);
    }

    GameRecord record;
    record.date = date; // Use the provided date
    if (opponent == "ai") {
        record.result = QString("%1 vs AI (%2)")
        .arg(result == "win" ? "Win" : (result == "loss" ? "Loss" : "Draw"))
            .arg(difficulty);
    } else {
        record.result = QString("%1 vs %2")
        .arg(result == "win" ? "Win" : (result == "loss" ? "Loss" : "Draw"))
            .arg(opponent);
    }

    m_gameHistory.prepend(record);
    // Keep only the last 20 games
    while (m_gameHistory.size() > 20) {
        m_gameHistory.removeLast();
    }
}

