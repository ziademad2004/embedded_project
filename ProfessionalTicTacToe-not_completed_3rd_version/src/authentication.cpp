#include "../include/authentication.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCryptographicHash>

Authentication::Authentication(QObject *parent)
    : QObject(parent), m_currentUser(nullptr)
{
    // Add some default users for demo purposes
    m_users.append(new User("player1", "pass123"));
    m_users.append(new User("player2", "pass123"));
    
    // Demo user removed
}

Authentication::~Authentication() {
    for (User* user : m_users) {
        delete user;
    }
}

bool Authentication::login(const QString &username, const QString &password) {
    for (User* user : m_users) {
        if (user->getUsername() == username && user->checkPassword(password)) {
            m_currentUser = user;
            emit loginStatusChanged(true, username);
            return true;
        }
    }
    m_currentUser = nullptr;
    emit loginStatusChanged(false, "");
    return false;
}

bool Authentication::registerUser(const QString &username, const QString &password) {
    // Check if user already exists
    for (User* user : m_users) {
        if (user->getUsername() == username) {
            return false;
        }
    }
    
    // Create new user
    m_users.append(new User(username, password));
    return true;
}

User* Authentication::getCurrentUser() const {
    return m_currentUser;
}

QVector<User*> Authentication::getUsers() const {
    return m_users;
}

bool Authentication::authenticatePlayers(const QString &player1Username, const QString &player1Password,
                                      const QString &player2Username, const QString &player2Password,
                                      QString &errorMessage) {
    User* player1 = nullptr;
    User* player2 = nullptr;
    
    for (User* user : m_users) {
        if (user->getUsername() == player1Username && user->checkPassword(player1Password)) {
            player1 = user;
        }
        if (user->getUsername() == player2Username && user->checkPassword(player2Password)) {
            player2 = user;
        }
    }
    
    if (!player1) {
        errorMessage = "Player 1: Invalid username or password";
        return false;
    }
    
    if (!player2) {
        errorMessage = "Player 2: Invalid username or password";
        return false;
    }
    
    if (player1Username == player2Username) {
        errorMessage = "Players must be different users";
        return false;
    }
    
    return true;
}

void Authentication::saveUsers() {
    // For simplicity, this method is not implemented
    // In a real application, you would save users to a file or database
}

void Authentication::loadUsers() {
    // For simplicity, this method is not implemented
    // In a real application, you would load users from a file or database
}
