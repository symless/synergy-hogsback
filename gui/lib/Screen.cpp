#include "Screen.h"

QHash<ScreenState, QString> Screen::m_stateImages;

Screen::Screen(QString name) :
	m_posX(-1),
	m_posY(-1),
	m_name(name),
	m_state(kIdle)
{
	m_stateImages[kReady] = "qrc:/res/image/screen-active.png";
	m_stateImages[kRunning] = "qrc:/res/image/screen_icon_running.png";
	m_stateImages[kIdle] = "qrc:/res/image/screen-inactive.png";

	m_stateImage = m_stateImages[m_state];
}

int Screen::posX() const
{
	return m_posX;
}

int Screen::posY() const
{
	return m_posY;
}

QString Screen::name() const
{
	return m_name;
}

ScreenState Screen::state() const
{
	return m_state;
}

QString Screen::stateImage() const
{
	return m_stateImage;
}

void Screen::setPosX(int x)
{
	m_posX = x;
}

void Screen::setPosY(int y)
{
	m_posY = y;
}

void Screen::setName(QString n)
{
	m_name = n;
}

void Screen::setState(ScreenState s)
{
	m_state = s;
	m_stateImage = m_stateImages[m_state];
}
