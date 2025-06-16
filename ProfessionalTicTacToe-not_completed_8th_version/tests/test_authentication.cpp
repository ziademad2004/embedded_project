// clazy:skip

#include <QTest>
#include <QObject>
#include "../include/authentication.h"

class TestAuthentication : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    
    void testPasswordHashing();
    void testPasswordVerification();
    void testSaltGeneration();
    void testUserRegistration();
    void testUserLogin();
    void testPlayerAuthentication();

private:
    Authentication *auth;
};

void TestAuthentication::init()
{
    auth = new Authentication();
}

void TestAuthentication::cleanup()
{
    delete auth;
}

void TestAuthentication::testPasswordHashing()
{
    QString password = "testPassword123";
    QString hash1 = Authentication::hashPassword(password);
    QString hash2 = Authentication::hashPassword(password);
    
    QVERIFY(hash1 != hash2); // Different due to salt
    QVERIFY(hash1.length() > password.length());
    QVERIFY(hash1.length() >= 80); // Salt + hash
}

void TestAuthentication::testPasswordVerification()
{
    QString password = "mySecretPassword";
    QString hash = Authentication::hashPassword(password);
    
    QVERIFY(Authentication::verifyPassword(password, hash));
    QVERIFY(!Authentication::verifyPassword("wrongPassword", hash));
    QVERIFY(!Authentication::verifyPassword("", hash));
}

void TestAuthentication::testSaltGeneration()
{
    QString salt1 = Authentication::generateSalt();
    QString salt2 = Authentication::generateSalt();
    
    QVERIFY(salt1 != salt2);
    QCOMPARE(salt1.length(), 16);
    QCOMPARE(salt2.length(), 16);
    
    QRegularExpression alphanumeric("^[A-Za-z0-9]+$");
    QVERIFY(alphanumeric.match(salt1).hasMatch());
    QVERIFY(alphanumeric.match(salt2).hasMatch());
}

void TestAuthentication::testUserRegistration()
{
    QString username = "newUser";
    QString password = "newPassword123";
    
    QVERIFY(auth->registerUser(username, password));
    QVERIFY(!auth->registerUser(username, password)); // Duplicate
}

void TestAuthentication::testUserLogin()
{
    QString username = "loginTestUser";
    QString password = "loginTestPass";
    
    QVERIFY(auth->registerUser(username, password));
    QVERIFY(auth->login(username, password));
    QCOMPARE(auth->getCurrentUser()->getUsername(), username);
    QVERIFY(!auth->login(username, "wrongPassword"));
}

void TestAuthentication::testPlayerAuthentication()
{
    QString player1 = "player1";
    QString password1 = "pass123";
    QString player2 = "player2";
    QString password2 = "pass123";
    QString errorMessage;
    
    QVERIFY(auth->authenticatePlayers(player1, password1, player2, password2, errorMessage));
    QVERIFY(!auth->authenticatePlayers(player1, "wrong", player2, password2, errorMessage));
    QVERIFY(!errorMessage.isEmpty());
}

QTEST_MAIN(TestAuthentication)
#include "test_authentication.moc"

