#include "MulticastMessageTest.h"

#include "MulticastMessage.h"
#include "ProcessMode.h"

MulticastMessageTest::MulticastMessageTest()
{

}

MulticastMessageTest::~MulticastMessageTest()
{

}

void MulticastMessageTest::parse_empty_setNothing()
{
	MulticastMessage msg("");
	int type = kUnknown;
	int mode = kUnknownMode;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
	QCOMPARE(msg.m_processMode, mode);
	QCOMPARE(msg.m_active, true);
	QCOMPARE(msg.m_joinGroup, false);
	QCOMPARE(msg.m_hostname.isEmpty(), true);
	QCOMPARE(msg.m_ip.isEmpty(), true);
	QCOMPARE(msg.m_uniqueGroup.isEmpty(), true);
	QCOMPARE(msg.m_configInfo.isEmpty(), true);
}

void MulticastMessageTest::parse_defaultExistence_setValidType()
{
	MulticastMessage msg("0");
	int type = kDefaultExistence;

	QCOMPARE(msg.m_valid, true);
	QCOMPARE(msg.m_type, type);
}

void MulticastMessageTest::parse_defaultExistence_setInvalidMessage()
{
	MulticastMessage msg("0,0");
	int type = kDefaultExistence;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
}

void MulticastMessageTest::parse_defaultReply_setValidTypeActiveUniqueGroup()
{
	MulticastMessage msg("1,1,1");
	int type = kDefaultReply;

	QCOMPARE(msg.m_valid, true);
	QCOMPARE(msg.m_type, type);
	QCOMPARE(msg.m_active, true);
	QCOMPARE(msg.m_uniqueGroup, QString("1"));
}

void MulticastMessageTest::parse_defaultReply_setInvalidMessage()
{
	MulticastMessage msg("1,1,1,1");
	int type = kDefaultReply;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
}

void MulticastMessageTest::parse_uniqueJoin_setValidTypeModeName()
{
	MulticastMessage msg("2,1,mock");
	int type = kUniqueJoin;
	int mode = kClientMode;

	QCOMPARE(msg.m_valid, true);
	QCOMPARE(msg.m_type, type);
	QCOMPARE(msg.m_processMode, mode);
	QCOMPARE(msg.m_hostname, QString("mock"));
}

void MulticastMessageTest::parse_uniqueJoin_setInvalidMessage()
{
	MulticastMessage msg("2,1,mock,1");
	int type = kUniqueJoin;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
}

void MulticastMessageTest::parse_uniqueLeave_setValidTypeModeNameIp()
{
	MulticastMessage msg("3,0,mockName,mockIp");
	int type = kUniqueLeave;
	int mode = kServerMode;

	QCOMPARE(msg.m_valid, true);
	QCOMPARE(msg.m_type, type);
	QCOMPARE(msg.m_processMode, mode);
	QCOMPARE(msg.m_hostname, QString("mockName"));
	QCOMPARE(msg.m_ip, QString("mockIp"));
}

void MulticastMessageTest::parse_uniqueLeave_setInvalidMessage()
{
	MulticastMessage msg("3,0,mockName,mockIp,1");
	int type = kUniqueLeave;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
}

void MulticastMessageTest::parse_uniqueClaim_setValidTypeIp()
{
	MulticastMessage msg("4,mock");
	int type = kUniqueClaim;

	QCOMPARE(msg.m_valid, true);
	QCOMPARE(msg.m_type, type);
	QCOMPARE(msg.m_ip, QString("mock"));
}

void MulticastMessageTest::parse_uniqueClaim_setInvalidMessage()
{
	MulticastMessage msg("4,0,mock,1");
	int type = kUniqueClaim;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
}

void MulticastMessageTest::parse_uniqueConfig_setValidTypeConfigInfo()
{
	MulticastMessage msg("5,mock");
	int type = kUniqueConfig;

	QCOMPARE(msg.m_valid, true);
	QCOMPARE(msg.m_type, type);
	QCOMPARE(msg.m_configInfo, QString("mock"));
}

void MulticastMessageTest::parse_uniqueConfig_setInvalidMessage()
{
	MulticastMessage msg("5");
	int type = kUniqueConfig;

	QCOMPARE(msg.m_valid, false);
	QCOMPARE(msg.m_type, type);
}
