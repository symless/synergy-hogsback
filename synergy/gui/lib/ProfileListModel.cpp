#include "Profile.h"
#include "ProfileListModel.h"

ProfileListModel::ProfileListModel()
{
    m_profiles.emplace_back (1, QString("Home"));
    m_profiles.emplace_back (2, QString("Work"));
    dataChanged(createIndex (0, 1, quintptr(0)),  createIndex (1, 1, quintptr(1)));
}

int
ProfileListModel::rowCount (QModelIndex const& parent) const
{
    return 2;
}

QVariant
ProfileListModel::data (QModelIndex const& index, int const role) const
{
    QVariant variant;
    if (role == Qt::UserRole) {
        auto const& profile = m_profiles.at (index.internalId());
        variant.setValue (profile);
    }
    return variant;
}

QHash<int, QByteArray>
ProfileListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = QString("profile").toLocal8Bit();
    return roles;
}
