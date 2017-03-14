#ifndef SCREEN_H
#define SCREEN_H

#include "LibMacro.h"
#include "ScreenState.h"

#include <QString>
#include <QHash>

class LIB_SPEC Screen
{
public:
    friend inline
    uint qHash (Screen const& screen, uint seed = 0) noexcept {
        return qHash (screen.m_name, seed);
    }

    friend bool
    operator== (Screen const& s1, Screen const& s2) noexcept {
        return (s1.m_name == s2.m_name && s1.m_id == s2.m_id);
    }

	Screen(QString name = "");

    int id() const;
    void setId(int id);
	int posX() const;
	int posY() const;
	QString name() const;
	ScreenState state() const;
	QString stateImage() const;

	void setPosX(int x);
	void setPosY(int y);
	void setName(QString n);
	void setState(ScreenState s);

    bool getLocked() const;
    void setLocked(bool value);

private:
    int m_id;
    int m_posX;
    int m_posY;
	QString m_name;
	ScreenState m_state;
	QString m_stateImage;
    bool locked;
    static QHash<ScreenState, QString> m_stateImages;
};

#endif // SCREEN_H
