#ifndef CONFIGMESSAGECONVERTOR_H
#define CONFIGMESSAGECONVERTOR_H

#include "Screen.h"
#include "LibMacro.h"

#include <QString>
#include <QList>

class ScreenModel;
class Screen;

class LIB_SPEC ConfigMessageConvertor
{
public:
	ConfigMessageConvertor();

	QString fromModelToString(ScreenModel* model);
	QString fromScreenToString(Screen& screen);
	bool fromStringToList(QList<Screen>& screens, QString& str,
			int& latestSerial, bool fullConfig);

};

#endif // CONFIGMESSAGECONVERTOR_H
