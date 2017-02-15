#include "ConfigMessageConvertor.h"

#include "ScreenListModel.h"
#include "Screen.h"

const QString kSeperator = ",";

ConfigMessageConvertor::ConfigMessageConvertor()
{

}

QString ConfigMessageConvertor::fromModelToString(ScreenListModel* model)
{
	// TODO: add multiple messages support
	QString result;

	int screenCount = model->m_screens.count();
	result += QString::number(screenCount);

	for (int i = 0; i < screenCount; i++) {
		const Screen& screen = model->getScreen(i);
		result += ',';
		result += screen.name();
		result += ',';
		result += QString::number(screen.posX());
		result += ',';
		result += QString::number(screen.posY());
		result += ',';
		result += QString::number(screen.state());
	}

	return result;
}

QString ConfigMessageConvertor::fromScreenToString(Screen& screen)
{
	QString result;

	result += screen.name();
	result += ',';
	result += QString::number(screen.posX());
	result += ',';
	result += QString::number(screen.posY());
	result += ',';
	result += QString::number(screen.state());

	return result;
}

bool ConfigMessageConvertor::fromStringToList(QList<Screen>& screens,
								QString& str, int& latestSerial,
								bool fullConfig)
{
	QStringList elements;
	elements = str.split(kSeperator);

	if (fullConfig) {
		if (elements.size() < 2) {
			return false;
		}

		int serialNumber = elements[0].toInt();
		if (serialNumber < latestSerial) {
			return false;
		}
		latestSerial = serialNumber;

		if (serialNumber > latestSerial) {
			screens.clear();
		}

		int screenCount = elements[1].toInt();
		int index = 2;

		for (int i = 0; i < screenCount; i++) {
			Screen screen;
			if (index + 4 > elements.size()) {
				break;
			}
			screen.setName(elements[index]);
			screen.setPosX(elements[index + 1].toInt());
			screen.setPosY(elements[index + 2].toInt());
			screen.setState(static_cast<ScreenState>(
								elements[index + 3].toInt()));

			screens.append(screen);
			index += 4;
		}
	}
	else {
		if (elements.size() != 4) {
			return false;
		}
		screens.clear();

		Screen screen;
		screen.setName(elements[0]);
		screen.setPosX(elements[1].toInt());
		screen.setPosY(elements[2].toInt());
		screen.setState(static_cast<ScreenState>(elements[3].toInt()));

		screens.append(screen);
	}

	return true;
}
