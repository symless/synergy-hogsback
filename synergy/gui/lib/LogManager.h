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
    ~LogManager();

    Q_INVOKABLE void uploadLogFile();
    Q_INVOKABLE static void setCloudClient(CloudClient* value);

    static void raw(const QString& text);
    static void error(const QString& text);
    static void warning(const QString& text);
    static void info(const QString& text);
    static void debug(const QString& text);
    static QString logFilename();
    static void setQmlContext(QQmlContext* value);

private:
    LogManager();

    static QString timeStamp();
	static void appendRaw(const QString& text);
    static void updateLogLineModel();

    bool getLastLines(int lines, QString& content);
    QString prepareUploadFile();
    QString generateLogFilename();
    void upload(QString filename);

private:
	static QFile s_file;
    static QStringList s_logLines;
	static QObject* s_instance;
    static QQmlContext* s_qmlContext;
    static CloudClient* s_cloudClient;
    static const int s_maximumLogLines;
    static bool s_uploading;
    static const int s_maximumUploadLogLines;
};

#endif // LOGMANAGER_H
