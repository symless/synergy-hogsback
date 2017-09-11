#ifndef SCREENLISTSNAPSHOT_H
#define SCREENLISTSNAPSHOT_H

#include "UIScreen.h"

#include <QObject>
#include <QSet>
#include <QList>
#include <QString>

class ScreenListModel;

/*class ScreenListSnapshot {
public:
    friend inline
    uint qHash (ScreenListSnapshot const& snapshot, uint seed) {
        return qHashRange(snapshot.m_screens.cbegin(),
                          snapshot.m_screens.cend(), seed);
    }
    auto size() const { return m_screens.size(); }
private:
    QList<Screen> m_screens;
};

inline
bool operator== (ScreenListSnapshot const&,
                 ScreenListSnapshot const&) noexcept {
    return false;
}
*/

class ScreenListSnapshotManager : public QObject
{
    Q_OBJECT
public:
    using SnapshotIndex = QSet<QSet<UIScreen>>::const_iterator;
    explicit ScreenListSnapshotManager(QObject* parent = 0);

    bool exactMatch(const QSet<UIScreen>& screenNames, SnapshotIndex& index) const;
    QList<UIScreen> getSnapshot(SnapshotIndex) const;
    void saveToFile();
    void loadFromFile();
    void saveSnapshot(ScreenListModel* screenListModel);

private:
    QSet<QSet<UIScreen>> m_snapshots;
};

#endif // SCREENLISTSNAPSHOT_H
