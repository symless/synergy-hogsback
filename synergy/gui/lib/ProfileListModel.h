#ifndef PROFILELISTMODEL_H
#define PROFILELISTMODEL_H

#include <QAbstractListModel>
#include <Profile.h>
#include <vector>

class ProfileListModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        ID = Qt::UserRole,
        Name
    };

    ProfileListModel();
    ProfileListModel(ProfileListModel const& src): m_profiles(src.m_profiles) {}

    ProfileListModel&
    operator=(ProfileListModel const& src) {
        m_profiles = src.m_profiles;
        return *this;
    }

    Q_INVOKABLE virtual int rowCount (QModelIndex const& parent = QModelIndex()) const override;
    Q_INVOKABLE virtual QVariant data (QModelIndex const&, int) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int add();
    Q_INVOKABLE void pop();

    void loadFromMap (QMap<QString, int> groupMap);

private:
    std::vector<Profile> m_profiles;
};

#endif // PROFILELISTMODEL_H
