#include "../include/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("Professional Tic Tac Toe");
    QApplication::setOrganizationName("TicTacToe Studios");
    QApplication::setOrganizationDomain("tictactoestudios.com");

    // Load application stylesheet
    QFile styleFile(":/styles/main.qss");
    if (styleFile.exists() && styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
