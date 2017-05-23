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

    void setVersion(const QString v);
    void checkUpdate(const QString& newVersion);
protected:
    VersionManager();

signals:
    void newVersionDetected(QString newVersion);

private:
    QString m_version;
};

#endif // VERSIONMANAGER_H
