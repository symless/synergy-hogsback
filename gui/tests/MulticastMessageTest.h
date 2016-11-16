#include "AutoTest.h"

class MulticastMessageTest : public QObject
{
	Q_OBJECT
public:
	MulticastMessageTest();
	~MulticastMessageTest();

private slots:
	void parse_empty_setNothing();
	void parse_defaultExistence_setValidType();
	void parse_defaultExistence_setInvalidMessage();
	void parse_defaultReply_setValidTypeActiveUniqueGroup();
	void parse_defaultReply_setInvalidMessage();
	void parse_uniqueJoin_setValidTypeModeName();
	void parse_uniqueJoin_setInvalidMessage();
	void parse_uniqueLeave_setValidTypeModeNameIp();
	void parse_uniqueLeave_setInvalidMessage();
	void parse_uniqueClaim_setValidTypeIp();
	void parse_uniqueClaim_setInvalidMessage();
	void parse_uniqueConfig_setValidTypeConfigInfo();
	void parse_uniqueConfig_setInvalidMessage();
};

DECLARE_TEST(MulticastMessageTest)
