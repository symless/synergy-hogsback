#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "LibMacro.h"

#include <QObject>
#include <QQmlEngine>
#include <QFile>
#include <QStringList>

class QQmlContext;
class CloudClient;

class LIB_SPEC LogManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(LogManager)

public:
    static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);
    LogManager();
    ~LogManager();

    Q_PROPERTY(QString dialogUrl READ dialogUrl NOTIFY dialogChanged)
    Q_PROPERTY(QString dialogText READ dialogText NOTIFY dialogChanged)
    Q_PROPERTY(bool dialogVisible READ dialogVisible NOTIFY dialogChanged)
    Q_PROPERTY(bool gdprAccepted READ gdprAccepted NOTIFY dialogChanged)

    Q_INVOKABLE void uploadLogFile();
    Q_INVOKABLE static void setCloudClient(CloudClient* value);
    Q_INVOKABLE void acceptGDPR();
    Q_INVOKABLE void dismissDialog();

    static void raw(const QString& text);
    static void error(const QString& text);
    static void warning(const QString& text);
    static void info(const QString& text);
    static void debug(const QString& text);
    static QString logFilename();
    static void setQmlContext(QQmlContext* value);
    QString dialogText() const;
    QString dialogUrl() const;
    bool dialogVisible() const;
    bool gdprAccepted() const;
    void setDialogUrl(const QString &dirty);

signals:
    void dialogChanged();
    void logLine(const QString& logLine);

private:
    static QString timeStamp();
    static void appendRaw(const QString& text, bool signal = true, bool tag = true);
    static void updateLogLineModel();

    bool getLastLines(int lines, QString& content);
    QString prepareUploadFile();
    QString generateLogFilename();
    void upload(QString filename);

private:
    static QStringList s_logLines;
    static QObject* s_instance;
    static QQmlContext* s_qmlContext;
    static CloudClient* s_cloudClient;
    static const int s_maximumLogLines;
    static bool s_uploading;
    static bool s_gdpr_accept;
    static const int s_maximumUploadLogLines;
    QString m_dialogText = "";
    QString m_dialogUrl = "";
    bool m_dialogVisible = false;
};

#endif // LOGMANAGER_H
