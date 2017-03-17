#include "LogManager.h"

#include "DirectoryManager.h"
#include "Common.h"

#include <QDateTime>
#include <QTextStream>
#include <QDebug>

QObject* LogManager::s_instance = NULL;
QFile LogManager::s_file;

const QString kDefaultLogFile = "synergy.log";

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
    s_file.open(QIODevice::WriteOnly);
}

LogManager::~LogManager()
{
    if (s_file.isOpen()) {
        s_file.close();
    }
}

void LogManager::row(const QString& text)
{
    appendRaw(text);
}
void LogManager::error(const QString& text)
{
    appendRaw(timeStamp() + " ERROR: " + text);
}

void LogManager::warning(const QString& text)
{
    appendRaw(timeStamp() + " WARNNIG: " + text);
}

void LogManager::info(const QString& text)
{
    appendRaw(timeStamp() + " INFO: " + text);
}

void LogManager::debug(const QString& text)
{

    appendRaw(timeStamp() + " DEBUG: " + text);
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
            // QTextStream stream(&s_file);
            // stream << line << endl;
            qDebug() << line << endl;
        }
    }
}
