#include "../include/authentication.h"
#include "../include/database.h"
#include <QCryptographicHash>
#include <QDateTime>

// Initialize static instance
Authentication* Authentication::instance = nullptr;

Authentication& Authentication::getInstance()
{
    if (!instance) {
        instance = new Authentication();
    }
    return *instance;
}

Authentication::Authentication(QObject *parent)
    : QObject(parent), loggedIn(false)
{
}

Authentication::~Authentication()
{
}

bool Authentication::registerUser(const QString &username, const QString &email, const QString &password)
{
    // Check if username is available
    if (!Database::getInstance().getUserByUsername(username).isValid()) {
        // Hash the password
        QString passwordHash = hashPassword(password);

        // Add user to database
        bool success = Database::getInstance().addUser(username, email, passwordHash);

        if (success) {
            emit registrationSuccessful();
        } else {
            emit registrationFailed("Database error during registration");
        }

        return success;
    } else {
        emit registrationFailed("Username already exists");
        return false;
    }
}

bool Authentication::loginUser(const QString &username, const QString &password)
{
    // Get stored password hash
    QString storedHash = Database::getInstance().getPasswordHash(username);

    if (storedHash.isEmpty()) {
        emit loginFailed("User does not exist");
        return false;
    }

    // Verify password
    if (verifyPassword(password, storedHash)) {
        // Get user information
        currentUser = Database::getInstance().getUserByUsername(username);

        // Update last login
        Database::getInstance().updateLastLogin(currentUser.getId(), QDateTime::currentDateTime());

        loggedIn = true;
        emit loginSuccessful(currentUser);
        return true;
    } else {
        emit loginFailed("Incorrect password");
        return false;
    }
}

void Authentication::logoutUser()
{
    if (loggedIn) {
        loggedIn = false;
        currentUser = User(); // Reset current user
        emit logoutSuccessful();
    }
}

User Authentication::getCurrentUser() const
{
    return currentUser;
}

bool Authentication::isUserLoggedIn() const
{
    return loggedIn;
}

QString Authentication::hashPassword(const QString &password)
{
    // Use SHA-256 for password hashing
    // In a real-world application, you should use a more secure approach with salt
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool Authentication::verifyPassword(const QString &plainPassword, const QString &hashedPassword)
{
    // Hash the provided password and compare with stored hash
    QString hash = hashPassword(plainPassword);
    return hash == hashedPassword;
}

User Authentication::getUserByUsername(const QString &username) const
{
    return Database::getInstance().getUserByUsername(username);
}

bool Authentication::verifyCredentials(const QString &username, const QString &password) const
{
    // Get stored password hash
    QString storedHash = Database::getInstance().getPasswordHash(username);

    if (storedHash.isEmpty()) {
        return false;
    }

    // Verify password
    // Note that we need to call hashPassword in a special way since we're in a const method
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    QString passwordHash = QString(hash.toHex());

    return passwordHash == storedHash;
}
