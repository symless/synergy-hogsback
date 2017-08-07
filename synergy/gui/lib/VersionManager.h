#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <QQmlEngine>
#include <QObject>

class VersionManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VersionManager)

public:
    static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);
    ~VersionManager();

    Q_INVOKABLE QString buildVersion() const;
    Q_INVOKABLE QString latestVersion() const;

    void setVersion(const QString v);
    void checkUpdate(QJsonDocument& updateReplyDoc);
    QString currentVersion() const;

protected:
    VersionManager();

signals:
    void newVersionDetected(QString newVersion);

private:
    QString m_version;
    QString m_latestVersion;
};

#endif // VERSIONMANAGER_H
