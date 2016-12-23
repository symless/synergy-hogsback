#include "ScreenListSnapshotManager.h"

#include "ScreenListModel.h"

ScreenListSnapshotManager::ScreenListSnapshotManager(QObject* parent) :
    QObject(parent)
{
}

ScreenListSnapshotManager::SnapshotIndex
ScreenListSnapshotManager::exactMatch(QSet<Screen> const& screens) const
{
    return m_snapshots.find(screens);
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

void ScreenListSnapshotManager::update(ScreenListModel *screenListModel)
{

}
