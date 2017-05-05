#ifndef PROFILELISTMODEL_H
#define PROFILELISTMODEL_H

#include <QAbstractListModel>
#include <Profile.h>
#include <vector>

class ProfileListModel: public QAbstractListModel
{
    Q_OBJECT
public:
    ProfileListModel();
    Q_INVOKABLE virtual int rowCount (QModelIndex const& parent = QModelIndex()) const override;
    Q_INVOKABLE virtual QVariant data (QModelIndex const&, int) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<Profile> m_profiles;
};

#endif // PROFILELISTMODEL_H
