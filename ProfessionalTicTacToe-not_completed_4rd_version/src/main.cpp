#include "../include/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Load and apply the stylesheet
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = styleFile.readAll();
        a.setStyleSheet(style);
        styleFile.close();
    }
    
    MainWindow w;
    w.setWindowTitle("Professional Tic Tac Toe");
    w.resize(900, 700);
    w.show();
    
    return a.exec();
}
