#ifndef COREINTERFACE_H
#define COREINTERFACE_H

#include <QString>

class CoreInterface
{
public:
	CoreInterface();

	QString profileDir();
	QString run(const QStringList& args, const QString& input = "");
};

#endif // COREINTERFACE_H
