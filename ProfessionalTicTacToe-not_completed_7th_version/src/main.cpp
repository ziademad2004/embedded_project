#include "../include/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Output welcome message
    qDebug() << "\n==========================================";
    qDebug() << "Welcome to Professional Tic Tac Toe Game!";
    qDebug() << "==========================================\n";
    qDebug() << "Game is starting...";
    qDebug() << "Loading resources and initializing...\n";
    
    // Load and apply the stylesheet
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = styleFile.readAll();
        a.setStyleSheet(style);
        styleFile.close();
        qDebug() << "✓ Stylesheet loaded successfully";
    } else {
        qDebug() << "! Warning: Could not load stylesheet";
    }
    
    MainWindow w;
    w.setWindowTitle("Professional Tic Tac Toe");
    w.resize(900, 700);
    w.show();
    
    qDebug() << "✓ Game window initialized and displayed";
    qDebug() << "✓ Game is ready to play!\n";
    
    return a.exec();
}
