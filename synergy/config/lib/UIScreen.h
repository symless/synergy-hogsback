#pragma once

#include "LibMacro.h"
#include "synergy/common/ScreenStatus.h"
#include "synergy/common/ErrorMessage.h"

#include <QString>
#include <QHash>

inline
int qHash(ScreenStatus key, uint seed = 0) noexcept {
    return qHash(static_cast<int>(key), seed);
}

// TODO: Unify this with the Screen class in common
class LIB_SPEC UIScreen
{
public:
    friend inline
    uint qHash (UIScreen const& screen, uint seed = 0) noexcept {
        return qHash (screen.m_name, seed);
    }

    friend bool
    operator== (UIScreen const& s1, UIScreen const& s2) noexcept {
        return (s1.m_name == s2.m_name && s1.m_id == s2.m_id);
    }

    UIScreen(QString name = "");

    int id() const;
    void setId(int id);
	int posX() const;
	int posY() const;
    QString name() const;
    QString statusImage() const;
    QString lastErrorMessage() const;
    QString helpLink() const;

	void setPosX(int x);
	void setPosY(int y);
	void setName(QString n);
    void setStatus(ScreenStatus s);
    void setStatus(QString s);
    void setLastErrorCode(const ErrorCode &lastErrorCode);

    bool locked() const;
    void setLocked(bool value);

    ScreenStatus status() const;
    ErrorCode lastErrorCode() const;

    int version() const;
    void setVersion(int version);
    void touch();

private:
    int m_id;
    int m_posX;
    int m_posY;
    QString m_name;
    ScreenStatus m_status;
    QString m_statusImage;
    bool m_locked;
    static QHash<ScreenStatus, QString> m_statusImages;
    ErrorCode m_lastErrorCode;
    int m_version;
};
