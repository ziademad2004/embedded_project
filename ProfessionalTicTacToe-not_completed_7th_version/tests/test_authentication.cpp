#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <qregularexpression.h>
#include "authentication.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>
class AuthenticationTest : public ::testing::Test {
protected:
    void SetUp() override {
        auth = new Authentication();
    }

    void TearDown() override {
        delete auth;
    }

    Authentication* auth;
};

TEST_F(AuthenticationTest, PasswordHashing) {
    QString password = "testPassword123";
    QString hash1 = Authentication::hashPassword(password);
    QString hash2 = Authentication::hashPassword(password);

    // Hashes should be different due to salt
    EXPECT_NE(hash1.toStdString(), hash2.toStdString());

    // Hash should be longer than original password
    EXPECT_GT(hash1.length(), password.length());

    // Hash should contain salt + hex hash (at least 80 chars)
    EXPECT_GE(hash1.length(), 80);
}

TEST_F(AuthenticationTest, PasswordVerification) {
    QString password = "mySecretPassword";
    QString hash = Authentication::hashPassword(password);

    // Correct password should verify
    EXPECT_TRUE(Authentication::verifyPassword(password, hash));

    // Wrong password should not verify
    EXPECT_FALSE(Authentication::verifyPassword("wrongPassword", hash));

    // Empty password should not verify
    EXPECT_FALSE(Authentication::verifyPassword("", hash));
}

TEST_F(AuthenticationTest, SaltGeneration) {
    QString salt1 = Authentication::generateSalt();
    QString salt2 = Authentication::generateSalt();

    // Salts should be different
    EXPECT_NE(salt1.toStdString(), salt2.toStdString());

    // Salt should be 16 characters long
    EXPECT_EQ(salt1.length(), 16);
    EXPECT_EQ(salt2.length(), 16);

    // Salt should only contain alphanumeric characters
    QRegularExpression alphanumeric("^[A-Za-z0-9]+$");
    EXPECT_TRUE(alphanumeric.match(salt1).hasMatch());
    EXPECT_TRUE(alphanumeric.match(salt2).hasMatch());
}

TEST_F(AuthenticationTest, UserRegistration) {
    QString username = "newUser";
    QString password = "newPassword123";

    // Registration should succeed
    EXPECT_TRUE(auth->registerUser(username, password));

    // Duplicate registration should fail
    EXPECT_FALSE(auth->registerUser(username, password));
}

TEST_F(AuthenticationTest, UserLogin) {
    QString username = "loginTestUser";
    QString password = "loginTestPass";

    // Register user first
    EXPECT_TRUE(auth->registerUser(username, password));

    // Login should work with correct credentials
    EXPECT_TRUE(auth->login(username, password));
    EXPECT_EQ(auth->getCurrentUser()->getUsername().toStdString(), username.toStdString());

    // Login should fail with wrong password
    EXPECT_FALSE(auth->login(username, "wrongPassword"));
}

TEST_F(AuthenticationTest, PlayerAuthentication) {
    QString player1 = "player1";
    QString password1 = "pass1";
    QString player2 = "player2";
    QString password2 = "pass2";
    QString errorMessage;

    // Register both players
    auth->registerUser(player1, password1);
    auth->registerUser(player2, password2);

    // Authentication should succeed
    EXPECT_TRUE(auth->authenticatePlayers(player1, password1, player2, password2, errorMessage));

    // Authentication should fail with wrong credentials
    EXPECT_FALSE(auth->authenticatePlayers(player1, "wrong", player2, password2, errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());

    // Authentication should fail with same user
    EXPECT_FALSE(auth->authenticatePlayers(player1, password1, player1, password1, errorMessage));
}

TEST_F(AuthenticationTest, GetUsers) {
    QString username1 = "user1";
    QString username2 = "user2";
    QString password = "testPass";

    // Initially should have default users
    QVector<User*> initialUsers = auth->getUsers();
    int initialCount = initialUsers.size();

    // Register new users
    auth->registerUser(username1, password);
    auth->registerUser(username2, password);

    // Should have more users now
    QVector<User*> newUsers = auth->getUsers();
    EXPECT_EQ(newUsers.size(), initialCount + 2);
}

TEST_F(AuthenticationTest, CurrentUserManagement) {
    QString username = "currentUser";
    QString password = "currentPass";

    // Initially no current user
    EXPECT_EQ(auth->getCurrentUser(), nullptr);

    // Register and login
    auth->registerUser(username, password);
    auth->login(username, password);

    // Should have current user
    EXPECT_NE(auth->getCurrentUser(), nullptr);
    EXPECT_EQ(auth->getCurrentUser()->getUsername().toStdString(), username.toStdString());
}

