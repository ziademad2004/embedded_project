#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QObject>
#include <QVector>
#include <QPair>
#include "user.h"

class Authentication : public QObject {
    Q_OBJECT

public:
    explicit Authentication(QObject *parent = nullptr);
    ~Authentication();

    bool login(const QString &username, const QString &password);
    bool registerUser(const QString &username, const QString &password);
    User* getCurrentUser() const;
    QVector<User*> getUsers() const;
    bool authenticatePlayers(const QString &player1Username, const QString &player1Password,
                          const QString &player2Username, const QString &player2Password,
                          QString &errorMessage);
    
    void saveUsers();
    void loadUsers();

signals:
    void loginStatusChanged(bool loggedIn, const QString &username);

private:
    QVector<User*> m_users;
    User* m_currentUser;
};

#endif // AUTHENTICATION_H
