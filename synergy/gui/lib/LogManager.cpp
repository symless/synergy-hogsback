#include "LogManager.h"

#include "DirectoryManager.h"
#include "Common.h"

#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include <QQmlContext>

QObject* LogManager::s_instance = NULL;
QQmlContext* LogManager::s_qmlContext = NULL;
QFile LogManager::s_file;
QStringList LogManager::s_logLines;
int LogManager::s_maximumLogLines = 100;

const QString kDefaultLogFile = "synergy.log";
const QString kGUILogPrefix = "[ UI ] ";
QObject* LogManager::instance(QQmlEngine* engine,
                                        QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static LogManager s_instance;
    return &s_instance;
}

LogManager::LogManager()
{
    DirectoryManager directoryManager;
    s_file.setFileName(directoryManager.profileDir() + '/'+ kDefaultLogFile);
    s_file.open(QIODevice::WriteOnly | QIODevice::Append);
}

LogManager::~LogManager()
{
    if (s_file.isOpen()) {
        s_file.close();
    }
}

void LogManager::raw(const QString& text)
{
    appendRaw(text);
}
void LogManager::error(const QString& text)
{
    appendRaw(kGUILogPrefix + timeStamp() + " ERROR: " + text);
}

void LogManager::warning(const QString& text)
{
    appendRaw(kGUILogPrefix + timeStamp() + " WARNNIG: " + text);
}

void LogManager::info(const QString& text)
{
    appendRaw(kGUILogPrefix + timeStamp() + " INFO: " + text);
}

void LogManager::debug(const QString& text)
{

    appendRaw(kGUILogPrefix + timeStamp() + " DEBUG: " + text);
}

QString LogManager::logFilename()
{
    return s_file.fileName();
}

QString LogManager::timeStamp()
{
    QDateTime current = QDateTime::currentDateTime();
    return '[' + current.toString(Qt::ISODate) + ']';
}

void LogManager::appendRaw(const QString& text)
{
    foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {
            QTextStream stream(&s_file);
            stream << line << endl;
            qDebug() << line << endl;

            if (s_logLines.size() > s_maximumLogLines) {
                s_logLines.pop_front();
            }

            s_logLines.push_back(line);
            updateLogLineModel();
        }
    }
}

void LogManager::updateLogLineModel()
{
    if (s_qmlContext == NULL) {
        return;
    }

    s_qmlContext->setContextProperty("LoggingModel", QVariant::fromValue(s_logLines));
}

void LogManager::setQmlContext(QQmlContext* value)
{
    s_qmlContext = value;

    updateLogLineModel();
}
