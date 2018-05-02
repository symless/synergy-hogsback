#include "LogManager.h"
#include "VersionManager.h"
#include "CloudClient.h"
#include "DirectoryManager.h"
#include "AppConfig.h"
#include "Common.h"
#include <synergy/common/DirectoryManager.h>

#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include <QQmlContext>

QObject* LogManager::s_instance = NULL;
QQmlContext* LogManager::s_qmlContext = NULL;
CloudClient* LogManager::s_cloudClient = NULL;
QStringList LogManager::s_logLines;
const int LogManager::s_maximumLogLines = 100;
bool LogManager::s_uploading = false;
bool LogManager::s_gdpr_accept = false;

const int LogManager::s_maximumUploadLogLines = 1000;

const char* kCombinedLogFile = "synergy-combined.log";
const QString kLogPrefix = "[ Config  ] ";

// TODO: Make LogManager thread safe
QObject* LogManager::instance(QQmlEngine* engine,
                                        QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static LogManager s_instance;
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);
    return &s_instance;
}

LogManager::LogManager()
{
}

LogManager::~LogManager()
{
}

void LogManager::uploadLogFile()
{
    if (!s_uploading && s_gdpr_accept) {

        m_dialogText = "Uploading log...";
        m_dialogUrl = "";
        m_dialogVisible = true;
        dialogChanged();

        info("Sending log file...");

        s_uploading = true;

        upload(logFilename());

        s_uploading = false;
    }
    else if (!s_gdpr_accept)
    {
        VersionManager* versionManager = qobject_cast<VersionManager*>(VersionManager::instance());


        m_dialogText = QString("By clicking send, you are accepting our <a href='https://symless.com/privacy-policy?source=s2-app&version=%1'>privacy policy</a>.").arg(versionManager->buildVersion());
        m_dialogUrl = "";
        m_dialogVisible = true;
        dialogChanged();
    }
}

void LogManager::raw(const QString& text)
{
    appendRaw(text, false, false);
}

void LogManager::error(const QString& text)
{
    appendRaw(timeStamp() + " error: " + text);
}

void LogManager::warning(const QString& text)
{
    appendRaw(timeStamp() + " warning: " + text);
}

void LogManager::info(const QString& text)
{
    appendRaw(timeStamp() + " info: " + text);
}

void LogManager::debug(const QString& text)
{

    appendRaw(timeStamp() + " debug: " + text);
}

QString LogManager::logFilename()
{
    auto path = DirectoryManager::instance()->systemLogDir() / kCombinedLogFile;
    return QString::fromStdString(path.string());
}

QString LogManager::timeStamp()
{
    QDateTime current = QDateTime::currentDateTime();
    return '[' + current.toString(Qt::ISODate) + ']';
}

void LogManager::appendRaw(const QString& text, bool signal, bool tag)
{
    LogManager* instance =
        qobject_cast<LogManager*>(LogManager::instance());

    QString prefix = tag ? kLogPrefix : "";

    foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {

            if (signal) {
                emit instance->logLine(line);
            }

            qDebug() << prefix << line << endl;

            if (s_logLines.size() > s_maximumLogLines) {
                s_logLines.pop_front();
            }

            s_logLines.push_back(prefix + line);
            updateLogLineModel();
        }
    }
}

void LogManager::updateLogLineModel()
{
    if (s_qmlContext == NULL) {
        return;
    }
    QString text = s_logLines.join('\n');
    s_qmlContext->setContextProperty("LoggingModel", QVariant::fromValue(text));
}

QString LogManager::generateLogFilename()
{
    // format: userID-timestamp.log
    // example: 1-2017-05-31T16-07-40.log

    AppConfig* appConfig =
            qobject_cast<AppConfig*>(AppConfig::instance());
    QString filename = QString::number(appConfig->userId());
    filename += '-';

    QDateTime current = QDateTime::currentDateTime();
    filename += current.toString(Qt::ISODate);
    filename += ".log";
    filename.replace(':', '-');

    return filename;
}

void LogManager::upload(QString filename)
{
    if (s_cloudClient) {
        s_cloudClient->uploadLogFile(filename, generateLogFilename());
    }
}

bool LogManager::dialogVisible() const
{
    return m_dialogVisible;
}

bool LogManager::gdprAccepted() const
{
    return s_gdpr_accept;
}

QString LogManager::dialogUrl() const
{
    return m_dialogUrl;
}

QString LogManager::dialogText() const
{
    return m_dialogText;
}

void LogManager::setDialogUrl(const QString &dirty)
{
    QString urlText = QString(dirty).replace("Upload path: ", "");
    QString url = QString("<a href='%1'>View log</a>").arg(urlText);

    m_dialogUrl = url;
    m_dialogText = "Log uploaded.";
    m_dialogVisible = true;
    dialogChanged();
}

void LogManager::setCloudClient(CloudClient* value)
{
    s_cloudClient = value;
}

void LogManager::dismissDialog()
{
    m_dialogVisible = false;
    dialogChanged();
}

void LogManager::acceptGDPR()
{
    s_gdpr_accept = true;
    dialogChanged();
}

void LogManager::setQmlContext(QQmlContext* value)
{
    s_qmlContext = value;

    updateLogLineModel();
}
