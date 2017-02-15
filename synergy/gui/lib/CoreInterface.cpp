#include "CoreInterface.h"

#include "DirectoryManager.h"

#include <QProcess>
#include <stdexcept>

static const char kCoreBinary[] = "syntool";

CoreInterface::CoreInterface()
{

}

QString CoreInterface::profileDir()
{
	QStringList args("--get-profile-dir");
	return run(args);
}

QString CoreInterface::run(const QStringList& args, const QString& input)
{
	DirectoryManager directoryManager;
	QString program(
		directoryManager.installedDir()
		+ "/" + kCoreBinary);

	QProcess process;
	process.setReadChannel(QProcess::StandardOutput);
	process.start(program, args);
	bool success = process.waitForStarted();

	QString output, error;
	if (success)
	{
		if (!input.isEmpty()) {
			process.write(input.toStdString().c_str());
		}

		if (process.waitForFinished()) {
			output = process.readAllStandardOutput().trimmed();
			error = process.readAllStandardError().trimmed();
		}
	}

	int code = process.exitCode();
	if (!error.isEmpty() || !success || code != 0)
	{
		throw std::runtime_error(
			QString("Code: %1\nError: %2")
				.arg(process.exitCode())
				.arg(error.isEmpty() ? "Unknown" : error)
				.toStdString());
	}

	return output;
}

