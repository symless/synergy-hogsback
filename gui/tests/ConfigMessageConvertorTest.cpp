#include "ConfigMessageConvertorTest.h"

#include "ScreenListModel.h"
#include "ConfigMessageConvertor.h"

ConfigMessageConvertorTest::ConfigMessageConvertorTest(QObject *parent) : QObject(parent)
{

}

void ConfigMessageConvertorTest::fromModelToString_emptyModel_returnZero()
{
	ScreenListModel model;
	ConfigMessageConvertor convertor;
	QString r = convertor.fromModelToString(&model);

	QCOMPARE(r, QString("0"));
}

void ConfigMessageConvertorTest::fromModelToString_validModel_returnAllScreensInfo()
{
	ScreenListModel model;
	Screen screen1("mock1");
	Screen screen2("mock2");
	model.addScreen(screen1);
	model.addScreen(screen2);
	ConfigMessageConvertor convertor;
	QString r = convertor.fromModelToString(&model);

	QCOMPARE(r, QString("2,mock1,-1,-1,2,mock2,-1,-1,2"));
}

void ConfigMessageConvertorTest::fromScreenToString_validScreen_returnScreenInfoString()
{
	Screen screen("mock");
	ConfigMessageConvertor convertor;
	QString r = convertor.fromScreenToString(screen);

	QCOMPARE(r, QString("mock,-1,-1,2"));
}

void ConfigMessageConvertorTest::fromStringToList_validFullConfigLatestSerial_updateScreenList()
{
	QString str("2,2,mock1,-1,-1,2,mock2,-1,-1,2");
	ConfigMessageConvertor convertor;
	QList<Screen> record;
	int latestSerial = 1;

	bool r = convertor.fromStringToList(record, str, latestSerial, true);

	QCOMPARE(r, true);
	QCOMPARE(record.size(), 2);
	QCOMPARE(record[0].name(), QString("mock1"));
	QCOMPARE(record[0].posX(), -1);
	QCOMPARE(record[0].posY(), -1);
	QCOMPARE(record[0].state(), kIdle);
	QCOMPARE(record[1].name(), QString("mock2"));\
	QCOMPARE(record[1].posX(), -1);
	QCOMPARE(record[1].posY(), -1);
	QCOMPARE(record[1].state(), kIdle);
}

void ConfigMessageConvertorTest::fromStringToList_validFullConfigOutdatedSerial_returnFalse()
{
	QString str("1,2,mock1,-1,-1,2,mock2,-1,-1,2");
	ConfigMessageConvertor convertor;
	QList<Screen> record;
	int latestSerial = 2;

	bool r = convertor.fromStringToList(record, str, latestSerial, true);

	QCOMPARE(r, false);
	QCOMPARE(record.size(), 0);
}

void ConfigMessageConvertorTest::fromStringToList_invalidFullConfig_returnFalse()
{
	QString str("2");
	ConfigMessageConvertor convertor;
	QList<Screen> record;
	int latestSerial = 1;

	bool r = convertor.fromStringToList(record, str, latestSerial, true);

	QCOMPARE(r, false);
	QCOMPARE(record.size(), 0);
}

void ConfigMessageConvertorTest::fromStringToList_validIncompleteConfigLatestSerial_updateScreenList()
{
	QString str("2,2,mock1,-1,-1,2,mock2,-1,-1");
	ConfigMessageConvertor convertor;
	QList<Screen> record;
	int latestSerial = 1;

	bool r = convertor.fromStringToList(record, str, latestSerial, true);

	QCOMPARE(r, true);
	QCOMPARE(record.size(), 1);
	QCOMPARE(record[0].name(), QString("mock1"));
	QCOMPARE(record[0].posX(), -1);
	QCOMPARE(record[0].posY(), -1);
	QCOMPARE(record[0].state(), kIdle);
}

void ConfigMessageConvertorTest::fromStringToList_validDeltaConfig_updateScreenList()
{
	QString str("mock1,-1,-1,2");
	ConfigMessageConvertor convertor;
	QList<Screen> record;
	int latestSerial = 1;

	bool r = convertor.fromStringToList(record, str, latestSerial, false);

	QCOMPARE(r, true);
	QCOMPARE(record.size(), 1);
	QCOMPARE(record[0].name(), QString("mock1"));
	QCOMPARE(record[0].posX(), -1);
	QCOMPARE(record[0].posY(), -1);
	QCOMPARE(record[0].state(), kIdle);
}

void ConfigMessageConvertorTest::fromStringToList_invalidDeltaConfig_returnFalse()
{
	QString str("mock1,-1,-1");
	ConfigMessageConvertor convertor;
	QList<Screen> record;
	int latestSerial = 1;

	bool r = convertor.fromStringToList(record, str, latestSerial, false);

	QCOMPARE(r, false);
	QCOMPARE(record.size(), 0);
}
