#ifndef SCREEN_H
#define SCREEN_H

#include "LibMacro.h"
#include "ScreenState.h"

#include <QString>
#include <QHash>

class LIB_SPEC Screen
{
public:
	Screen(QString name = "");

	int posX() const;
	int posY() const;
	QString name() const;
	ScreenState state() const;
	QString stateImage() const;

	void setPosX(int x);
	void setPosY(int y);
	void setName(QString n);
	void setState(ScreenState s);

private:
	int m_posX;
	int m_posY;
	QString m_name;
	ScreenState m_state;
	QString m_stateImage;
	QHash<ScreenState, QString> m_stateImages;
};

#endif // SCREEN_H
