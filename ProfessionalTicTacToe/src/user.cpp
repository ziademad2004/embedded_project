#include "../include/user.h"

User::User()
    : id(-1), gamesPlayed(0), wins(0), losses(0), draws(0)
{
}

User::User(int id, const QString &username, const QString &email)
    : id(id), username(username), email(email), gamesPlayed(0), wins(0), losses(0), draws(0)
{
}

int User::getId() const
{
    return id;
}

QString User::getUsername() const
{
    return username;
}

QString User::getEmail() const
{
    return email;
}

int User::getGamesPlayed() const
{
    return gamesPlayed;
}

int User::getWins() const
{
    return wins;
}

int User::getLosses() const
{
    return losses;
}

int User::getDraws() const
{
    return draws;
}

QDateTime User::getLastLogin() const
{
    return lastLogin;
}

void User::setId(int id)
{
    this->id = id;
}

void User::setUsername(const QString &username)
{
    this->username = username;
}

void User::setEmail(const QString &email)
{
    this->email = email;
}

void User::setGamesPlayed(int games)
{
    this->gamesPlayed = games;
}

void User::setWins(int wins)
{
    this->wins = wins;
}

void User::setLosses(int losses)
{
    this->losses = losses;
}

void User::setDraws(int draws)
{
    this->draws = draws;
}

void User::setLastLogin(const QDateTime &lastLogin)
{
    this->lastLogin = lastLogin;
}

void User::incrementGamesPlayed()
{
    gamesPlayed++;
}

void User::incrementWins()
{
    wins++;
    incrementGamesPlayed();
}

void User::incrementLosses()
{
    losses++;
    incrementGamesPlayed();
}

void User::incrementDraws()
{
    draws++;
    incrementGamesPlayed();
}

bool User::isValid() const
{
    return id >= 0 && !username.isEmpty();
} 