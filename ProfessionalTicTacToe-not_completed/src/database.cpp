#include "../include/database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>

// Initialize static instance
Database* Database::instance = nullptr;

Database& Database::getInstance()
{
    if (!instance) {
        instance = new Database();

        // If there was an initialization error, emit it now
        if (!instance->initErrorMessage.isEmpty()) {
            emit instance->databaseError(instance->initErrorMessage);
            instance->initErrorMessage.clear();
        }
    }
    return *instance;
}

Database::Database(QObject *parent)
    : QObject(parent)
{
    // Initialize database
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                     + "/tictactoe.db";

    // Ensure directory exists
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "Error opening database:" << db.lastError().text();
        initErrorMessage = "Failed to open database";
    } else {
        initializeDatabase();
    }
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::initializeDatabase()
{
    QSqlQuery query(db);

    // Create users table
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT UNIQUE NOT NULL, "
                    "email TEXT, "
                    "password_hash TEXT NOT NULL, "
                    "games_played INTEGER DEFAULT 0, "
                    "wins INTEGER DEFAULT 0, "
                    "losses INTEGER DEFAULT 0, "
                    "draws INTEGER DEFAULT 0, "
                    "last_login DATETIME)")) {
        qWarning() << "Error creating users table:" << query.lastError().text();
        emit databaseError("Failed to create users table");
        return false;
    }

    // Create games table
    if (!query.exec("CREATE TABLE IF NOT EXISTS games ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "user_id INTEGER NOT NULL, "
                    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                    "vs_ai BOOLEAN NOT NULL, "
                    "opponent TEXT, "
                    "result INTEGER, "
                    "FOREIGN KEY(user_id) REFERENCES users(id))")) {
        qWarning() << "Error creating games table:" << query.lastError().text();
        emit databaseError("Failed to create games table");
        return false;
    }

    // Create moves table
    if (!query.exec("CREATE TABLE IF NOT EXISTS moves ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "game_id INTEGER NOT NULL, "
                    "row INTEGER NOT NULL, "
                    "col INTEGER NOT NULL, "
                    "player INTEGER NOT NULL, "
                    "move_number INTEGER NOT NULL, "
                    "FOREIGN KEY(game_id) REFERENCES games(id))")) {
        qWarning() << "Error creating moves table:" << query.lastError().text();
        emit databaseError("Failed to create moves table");
        return false;
    }

    return true;
}

bool Database::executeSqlQuery(const QString &queryString)
{
    QSqlQuery query(db);
    if (!query.exec(queryString)) {
        qWarning() << "SQL Error:" << query.lastError().text();
        emit databaseError("SQL Query error: " + query.lastError().text());
        return false;
    }
    return true;
}

bool Database::addUser(const QString &username, const QString &email, const QString &passwordHash)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(email);
    query.addBindValue(passwordHash);

    if (!query.exec()) {
        qWarning() << "Error adding user:" << query.lastError().text();
        emit databaseError("Failed to add user");
        return false;
    }

    return true;
}

User Database::getUserById(int id) const
{
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec() || !query.next()) {
        qWarning() << "Error getting user by ID:" << query.lastError().text();
        return User();
    }

    User user(
        query.value("id").toInt(),
        query.value("username").toString(),
        query.value("email").toString()
        );

    user.setGamesPlayed(query.value("games_played").toInt());
    user.setWins(query.value("wins").toInt());
    user.setLosses(query.value("losses").toInt());
    user.setDraws(query.value("draws").toInt());

    if (!query.value("last_login").isNull()) {
        user.setLastLogin(query.value("last_login").toDateTime());
    }

    return user;
}

User Database::getUserByUsername(const QString &username) const
{
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        return User();
    }

    User user(
        query.value("id").toInt(),
        query.value("username").toString(),
        query.value("email").toString()
        );

    user.setGamesPlayed(query.value("games_played").toInt());
    user.setWins(query.value("wins").toInt());
    user.setLosses(query.value("losses").toInt());
    user.setDraws(query.value("draws").toInt());

    if (!query.value("last_login").isNull()) {
        user.setLastLogin(query.value("last_login").toDateTime());
    }

    return user;
}

QVector<User> Database::getAllUsers() const
{
    QVector<User> users;

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM users ORDER BY wins DESC")) {
        qWarning() << "Error getting all users:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        User user(
            query.value("id").toInt(),
            query.value("username").toString(),
            query.value("email").toString()
            );

        user.setGamesPlayed(query.value("games_played").toInt());
        user.setWins(query.value("wins").toInt());
        user.setLosses(query.value("losses").toInt());
        user.setDraws(query.value("draws").toInt());

        if (!query.value("last_login").isNull()) {
            user.setLastLogin(query.value("last_login").toDateTime());
        }

        users.append(user);
    }

    return users;
}

bool Database::updateUserStats(int userId, int wins, int losses, int draws)
{
    QSqlQuery query(db);
    query.prepare("UPDATE users SET wins = ?, losses = ?, draws = ?, games_played = ? WHERE id = ?");
    query.addBindValue(wins);
    query.addBindValue(losses);
    query.addBindValue(draws);
    query.addBindValue(wins + losses + draws);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Error updating user stats:" << query.lastError().text();
        emit databaseError("Failed to update user statistics");
        return false;
    }

    return true;
}

bool Database::updateLastLogin(int userId, const QDateTime &lastLogin)
{
    QSqlQuery query(db);
    query.prepare("UPDATE users SET last_login = ? WHERE id = ?");
    query.addBindValue(lastLogin);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Error updating last login:" << query.lastError().text();
        emit databaseError("Failed to update last login");
        return false;
    }

    return true;
}

QString Database::getPasswordHash(const QString &username) const
{
    QSqlQuery query(db);
    query.prepare("SELECT password_hash FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        return QString();
    }

    return query.value("password_hash").toString();
}

bool Database::saveGame(int userId, bool vsAI, const QString &opponent, GameLogic::GameState result, const std::vector<GameLogic::Move> &moves)
{
    // Start transaction
    db.transaction();

    // Insert game record
    QSqlQuery query(db);
    query.prepare("INSERT INTO games (user_id, vs_ai, opponent, result) VALUES (?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(vsAI);
    query.addBindValue(opponent);
    query.addBindValue(static_cast<int>(result));

    if (!query.exec()) {
        qWarning() << "Error saving game:" << query.lastError().text();
        db.rollback();
        emit databaseError("Failed to save game");
        return false;
    }

    // Get the new game ID
    int gameId = query.lastInsertId().toInt();

    // Insert moves
    int moveNumber = 0;
    for (const auto &move : moves) {
        query.prepare("INSERT INTO moves (game_id, row, col, player, move_number) VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(gameId);
        query.addBindValue(move.row);
        query.addBindValue(move.col);
        query.addBindValue(static_cast<int>(move.player));
        query.addBindValue(moveNumber++);

        if (!query.exec()) {
            qWarning() << "Error saving move:" << query.lastError().text();
            db.rollback();
            emit databaseError("Failed to save game moves");
            return false;
        }
    }

    // Update user statistics
    User user = getUserById(userId);
    if (user.isValid()) {
        int wins = user.getWins();
        int losses = user.getLosses();
        int draws = user.getDraws();

        if (result == GameLogic::GameState::XWins) {
            // User is X, so they won
            wins++;
        } else if (result == GameLogic::GameState::OWins) {
            // User is X, so they lost
            losses++;
        } else if (result == GameLogic::GameState::Draw) {
            draws++;
        }

        if (!updateUserStats(userId, wins, losses, draws)) {
            db.rollback();
            return false;
        }
    }

    // Commit transaction
    if (!db.commit()) {
        qWarning() << "Error committing transaction:" << db.lastError().text();
        db.rollback();
        emit databaseError("Failed to save game data");
        return false;
    }

    return true;
}

QVector<QPair<int, QDateTime>> Database::getUserGameIds(int userId) const
{
    QVector<QPair<int, QDateTime>> gameIds;

    QSqlQuery query(db);
    query.prepare("SELECT id, timestamp FROM games WHERE user_id = ? ORDER BY timestamp DESC");
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Error getting user game IDs:" << query.lastError().text();
        return gameIds;
    }

    while (query.next()) {
        gameIds.append(qMakePair(query.value("id").toInt(), query.value("timestamp").toDateTime()));
    }

    return gameIds;
}

QPair<GameLogic::GameState, std::vector<GameLogic::Move>> Database::getGameDetails(int gameId) const
{
    QPair<GameLogic::GameState, std::vector<GameLogic::Move>> result;

    // Get game state
    QSqlQuery gameQuery(db);
    gameQuery.prepare("SELECT result FROM games WHERE id = ?");
    gameQuery.addBindValue(gameId);

    if (!gameQuery.exec() || !gameQuery.next()) {
        qWarning() << "Error getting game details:" << gameQuery.lastError().text();
        return result;
    }

    result.first = static_cast<GameLogic::GameState>(gameQuery.value("result").toInt());

    // Get moves
    QSqlQuery movesQuery(db);
    movesQuery.prepare("SELECT row, col, player FROM moves WHERE game_id = ? ORDER BY move_number");
    movesQuery.addBindValue(gameId);

    if (!movesQuery.exec()) {
        qWarning() << "Error getting game moves:" << movesQuery.lastError().text();
        return result;
    }

    while (movesQuery.next()) {
        GameLogic::Move move;
        move.row = movesQuery.value("row").toInt();
        move.col = movesQuery.value("col").toInt();
        move.player = static_cast<GameLogic::Player>(movesQuery.value("player").toInt());

        result.second.push_back(move);
    }

    return result;
}
