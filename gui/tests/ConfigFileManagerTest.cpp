#include "ConfigFileManagerTest.h"

#include "ConfigFileManager.h"
#include "Screen.h"
#include "Common.h"

#include <QVector2D>

ConfigFileManagerTest::ConfigFileManagerTest(QObject* parent) : QObject(parent)
{

}

void ConfigFileManagerTest::calculatRelativePercentage_halfRightOverlap_intervalsModified()
{
	ConfigFileManager configFileManager(NULL, NULL);
	Screen src("mockScreen1");
	Screen des("mockScreen2");
	src.setPosX(0);
	src.setPosY(0);
	des.setPosX(kScreenIconWidth);
	des.setPosY(kScreenIconHeight / 2);
	QVector2D srcInterval(0, 100);
	QVector2D desInterval(0, 100);
	configFileManager.calculatRelativePercentage(src, des, kRight,
							srcInterval, desInterval);

	QCOMPARE(srcInterval.x(), 50.0f);
	QCOMPARE(srcInterval.y(), 100.0f);
	QCOMPARE(desInterval.x(), 0.0f);
	QCOMPARE(desInterval.y(), 50.0f);
}

void ConfigFileManagerTest::calculatRelativePercentage_quarterLeftOverlap_intervalsModified()
{
	ConfigFileManager configFileManager(NULL, NULL);
	Screen src("mockScreen1");
	Screen des("mockScreen2");
	src.setPosX(kScreenIconWidth);
	src.setPosY(kScreenIconHeight / 4 * 3);
	des.setPosX(0);
	des.setPosY(0);
	QVector2D srcInterval(0, 100);
	QVector2D desInterval(0, 100);
	configFileManager.calculatRelativePercentage(src, des, kLeft,
							srcInterval, desInterval);

	QCOMPARE(srcInterval.x(), 0.0f);
	QCOMPARE(srcInterval.y(), 25.0f);
	QCOMPARE(desInterval.x(), 75.0f);
	QCOMPARE(desInterval.y(), 100.0f);
}

void ConfigFileManagerTest::calculatRelativePercentage_halfUpOverlap_intervalsModified()
{
	ConfigFileManager configFileManager(NULL, NULL);
	Screen src("mockScreen1");
	Screen des("mockScreen2");
	src.setPosX(kScreenIconWidth / 2);
	src.setPosY(kScreenIconHeight);
	des.setPosX(0);
	des.setPosY(0);
	QVector2D srcInterval(0, 100);
	QVector2D desInterval(0, 100);
	configFileManager.calculatRelativePercentage(src, des, kUp,
							srcInterval, desInterval);

	QCOMPARE(srcInterval.x(), 0.0f);
	QCOMPARE(srcInterval.y(), 50.0f);
	QCOMPARE(desInterval.x(), 50.0f);
	QCOMPARE(desInterval.y(), 100.0f);
}

void ConfigFileManagerTest::calculatRelativePercentage_quarterDownOverlap_intervalsModified()
{
	ConfigFileManager configFileManager(NULL, NULL);
	Screen src("mockScreen1");
	Screen des("mockScreen2");
	src.setPosX(0);
	src.setPosY(0);
	des.setPosX(kScreenIconWidth / 4 * 3);
	des.setPosY(kScreenIconHeight);
	QVector2D srcInterval(0, 100);
	QVector2D desInterval(0, 100);
	configFileManager.calculatRelativePercentage(src, des, kUp,
							srcInterval, desInterval);

	QCOMPARE(srcInterval.x(), 75.0f);
	QCOMPARE(srcInterval.y(), 100.0f);
	QCOMPARE(desInterval.x(), 0.0f);
	QCOMPARE(desInterval.y(), 25.0f);
}

