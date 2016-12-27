#include "ScreenListSnapshotManager.h"

#include "ScreenListModel.h"

ScreenListSnapshotManager::ScreenListSnapshotManager(QObject* parent) :
    QObject(parent)
{
}

bool
ScreenListSnapshotManager::exactMatch(
            QSet<Screen> const& screens,
            SnapshotIndex& index) const
{
    index = m_snapshots.find(screens);
    bool found = true;

    if (index == m_snapshots.end()) {
        found = false;
    }

    return found;
}

QList<Screen> ScreenListSnapshotManager::getSnapshot(SnapshotIndex index) const
{
    return QList<Screen>::fromSet(*index);
}

void ScreenListSnapshotManager::saveToFile()
{

}

void ScreenListSnapshotManager::loadFromFile()
{

}

void ScreenListSnapshotManager::saveSnapshot(ScreenListModel* screenListModel)
{
    m_snapshots.insert(screenListModel->getScreenNames());
}
