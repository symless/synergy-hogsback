#ifndef CONFIGFILEMANAGER_H
#define CONFIGFILEMANAGER_H

#include "LibMacro.h"
#include "Direction.h"

#include <QString>

class QTextStream;
class ScreenListModel;
class IScreenArrangement;
class Screen;
class QVector2D;

class LIB_SPEC ConfigFileManager
{
	friend class ConfigFileManagerTest;
public:
	ConfigFileManager(ScreenListModel* screens,
		IScreenArrangement* arrangementStrategy);

	void writeConfigurationFile(QString path = "");

private:
	void writeDefaultScreenSettings(QTextStream& stream);
	void writeDefaultOptionSettings(QTextStream& stream);
	void writeLinkSection(QTextStream& stream);
	void writeRelativePosition(QTextStream& stream,
			const Screen& src, const Screen& des, Direction dir);
	void calculatRelativePercentage(const Screen& src, const Screen& des,
			Direction dir, QVector2D& srcInterval, QVector2D& desInterval);
private:
	ScreenListModel* m_screens;
	IScreenArrangement* m_arrangementStrategy;
};

#endif // CONFIGFILEMANAGER_H
