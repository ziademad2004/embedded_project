#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QObject>
#include "user.h"

class Authentication : public QObject
{
    Q_OBJECT

public:
    static Authentication& getInstance();
    
    bool registerUser(const QString &username, const QString &email, const QString &password);
    bool loginUser(const QString &username, const QString &password);
    void logoutUser();
    
    User getCurrentUser() const;
    bool isUserLoggedIn() const;
    
    User getUserByUsername(const QString &username) const;
    bool verifyPassword(const QString &username, const QString &password) const;

signals:
    void loginSuccessful(const User &user);
    void loginFailed(const QString &reason);
    void logoutSuccessful();
    void registrationSuccessful();
    void registrationFailed(const QString &reason);

private:
    Authentication(QObject *parent = nullptr);
    ~Authentication();
    
    // Disable copy constructor and assignment operator
    Authentication(const Authentication&) = delete;
    Authentication& operator=(const Authentication&) = delete;
    
    // We'll use a singleton pattern
    static Authentication* instance;
    
    // Password hashing and verification
    QString hashPassword(const QString &password);
    bool verifyPassword(const QString &plainPassword, const QString &hashedPassword);
    
    // Current user
    User currentUser;
    bool loggedIn;
};

#endif // AUTHENTICATION_H 