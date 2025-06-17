#ifndef GAMEHISTORY_H
#define GAMEHISTORY_H

#include <QObject>
#include <QVector>
#include <QString>

class GameHistory : public QObject {
    Q_OBJECT

public:
    struct HistoryEntry {
        QString date;
        QString result;
    };

    explicit GameHistory(QObject *parent = nullptr);
    
    void addEntry(const QString &result);
    QVector<HistoryEntry> getEntries() const;
    void clearHistory();

private:
    QVector<HistoryEntry> m_entries;
    static const int MAX_ENTRIES = 5;
};

#endif // GAMEHISTORY_H
