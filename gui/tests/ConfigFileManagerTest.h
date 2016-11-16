#include "AutoTest.h"

class ConfigFileManagerTest : public QObject
{
	Q_OBJECT
public:
	explicit ConfigFileManagerTest(QObject* parent = 0);

private slots:
	void calculatRelativePercentage_halfRightOverlap_intervalsModified();
	void calculatRelativePercentage_quarterLeftOverlap_intervalsModified();
	void calculatRelativePercentage_halfUpOverlap_intervalsModified();
	void calculatRelativePercentage_quarterDownOverlap_intervalsModified();
};

DECLARE_TEST(ConfigFileManagerTest)
