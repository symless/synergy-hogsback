#include "AutoTest.h"

#include <QObject>

class ProcessCommandTest : public QObject
{
    Q_OBJECT
public:
    explicit ProcessCommandTest(QObject* parent = 0);

private slots:
    void arguments_serverMode_validServerArgument();
    void arguments_clientMode_validClientArgument();
    void arguments_clientModeWithoutServerIP_emptyClientArgument();
};

DECLARE_TEST(ProcessCommandTest)
