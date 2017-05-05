#include "Profile.h"
#include "ProfileListModel.h"

ProfileListModel::ProfileListModel()
{
}

int
ProfileListModel::rowCount (QModelIndex const& parent) const
{
    return m_profiles.size();
}

QVariant
ProfileListModel::data (QModelIndex const& index, int const role) const
{
    QVariant var;
    auto const& profile = m_profiles.at (index.row());
    switch (role) {
        case Roles::ID:
            var.setValue (profile.id());
            break;
        case Roles::Name:
            var.setValue (profile.name());
            break;
    }
    return var;
}

QHash<int, QByteArray>
ProfileListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Roles::ID] = QString("profId").toLocal8Bit();
    roles[Roles::Name] = QString("profName").toLocal8Bit();
    return roles;
}

int
ProfileListModel::add()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_profiles.emplace_back();
    endInsertRows();
    return m_profiles.size() - 1;
}

void
ProfileListModel::pop()
{
    beginRemoveRows(QModelIndex(), rowCount() - 1, rowCount() - 1);
    m_profiles.pop_back();
    endRemoveRows();
}

void
ProfileListModel::loadFromMap(QMap<QString, int> groupMap)
{
    beginRemoveRows(QModelIndex(), 0, rowCount() ? rowCount() - 1 : 0);
    m_profiles.clear();
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, groupMap.size() ? groupMap.size() - 1 : 0);
    for (auto& key: groupMap.keys()) {
        m_profiles.emplace_back (-1, key); /* TODO: actually insert profile id */
    }
    endInsertRows();
}
