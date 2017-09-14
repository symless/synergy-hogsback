#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <ProfileListModel.h>
#include <CloudClient.h>

class ProfileManager: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProfileManager)

public:
    static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);

    Q_INVOKABLE ProfileListModel*
    listModel() {
        return new ProfileListModel (m_listModel);
    }

public slots:
    void updateProfiles (QMap<QString, int>);

protected:
    ProfileManager();

private:
    ProfileListModel m_listModel;
    CloudClient* m_cloudClient;
};

#endif // PROFILEMANAGER_H
