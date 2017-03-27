#include "ProcessCommandTest.h"

#include "ProcessCommand.h"
#include "MockDirectoryManager.h"

ProcessCommandTest::ProcessCommandTest(QObject* parent) : QObject(parent)
{

}

void ProcessCommandTest::arguments_serverMode_validServerArgument()
{
    ProcessCommand processCommand;
    MockDirectoryManager mockDirectoryManager;
    processCommand.setDirectoryManager(&mockDirectoryManager);
    QStringList args = processCommand.arguments(true);

#ifdef Q_OS_WIN
    QCOMPARE(args.count(), 23);
#else
    QCOMPARE(args.count(), 15);
#endif
}

void ProcessCommandTest::arguments_clientMode_validClientArgument()
{
    ProcessCommand processCommand;
    MockDirectoryManager mockDirectoryManager;
    processCommand.setDirectoryManager(&mockDirectoryManager);
    processCommand.setServerIp("1.1.1.1");
    QStringList args = processCommand.arguments(false);

    QCOMPARE(args.count(), 12);
}

void ProcessCommandTest::arguments_clientModeWithoutServerIP_emptyClientArgument()
{
    // empty client argument because of no server ip specified
    ProcessCommand processCommand;
    MockDirectoryManager mockDirectoryManager;
    processCommand.setDirectoryManager(&mockDirectoryManager);
    QStringList args = processCommand.arguments(false);

    QCOMPARE(args.count(), 0);
}
