#include "../include/gamehistory.h"
#include <QDateTime>

GameHistory::GameHistory(QObject *parent)
    : QObject(parent)
{
}

void GameHistory::addEntry(const QString &result) {
    HistoryEntry entry;
    entry.date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    entry.result = result;
    
    m_entries.prepend(entry);
    
    // Keep only the last MAX_ENTRIES
    while (m_entries.size() > MAX_ENTRIES) {
        m_entries.removeLast();
    }
}

QVector<GameHistory::HistoryEntry> GameHistory::getEntries() const {
    return m_entries;
}

void GameHistory::clearHistory() {
    m_entries.clear();
}
