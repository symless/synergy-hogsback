#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "LibMacro.h"

#include <QObject>
#include <QQmlEngine>
#include <QFile>

class LIB_SPEC LogManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(LogManager)

public:
	static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);

	static void row(const QString& text);
	static void error(const QString& text);
	static void warning(const QString& text);
	static void info(const QString& text);
	static void debug(const QString& text);

	~LogManager();

private:
	LogManager();

	static QString timeStamp();
	static void appendRaw(const QString& text);

private:
	static QFile s_file;

	static QObject* s_instance;
};

#endif // LOGMANAGER_H
