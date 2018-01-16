#include "UIScreen.h"
#include "LogManager.h"
#include "Common.h"

QHash<ScreenStatus, QString> UIScreen::m_statusImages = {
    {ScreenStatus::kConnected, "qrc:/res/image/screen-active.png"},
    {ScreenStatus::kConnecting, "qrc:/res/image/screen-inactive.png"},
    {ScreenStatus::kConnectingWithError, "qrc:/res/image/screen-inactive.png"},
    {ScreenStatus::kDisconnected, "qrc:/res/image/screen-inactive.png"},
    {ScreenStatus::kInactive, "qrc:/res/image/screen-inactive.png"}
};

UIScreen::UIScreen(QString name) :
    m_id(-1),
    m_posX(-1),
    m_posY(-1),
    m_name(name),
    m_status(ScreenStatus::kInactive),
    m_locked(false),
    m_lastErrorCode(kNoError),
    m_version(-1)
{
    m_statusImage = m_statusImages[m_status];
}

int UIScreen::posX() const
{
    return m_posX;
}

int UIScreen::posY() const
{
    return m_posY;
}

QString UIScreen::name() const
{
    return m_name;
}

QString UIScreen::statusImage() const
{
    return m_statusImage;
}

QString
UIScreen::lastErrorMessage() const
{
    return  QString::fromStdString(getErrorMessage(m_lastErrorCode));
}

QString
UIScreen::helpLink() const
{
    return  QString::fromStdString(getHelpUrl(m_lastErrorCode));
}

void UIScreen::setPosX(int const x)
{
    if ((x <= -kScreenIconWidth) || (x >= kDefaultViewWidth)) {
        LogManager::warning (QString("Attempted to set screen position x coordinate "
                                     "outside visible area. x = %1").arg(x));
        return;
    }

    m_posX = x;
}

void UIScreen::setPosY(int const y)
{
    if ((y <= -kScreenIconHeight) || (y >= kDefaultViewHeight)) {
        LogManager::warning (QString("Attempted to set screen position y coordinate "
                                     "outside visible area. y = %1").arg(y));
        return;
    }
    m_posY = y;
}

void UIScreen::setName(QString n)
{
    m_name = n;
}

void UIScreen::setStatus(ScreenStatus s)
{
    m_status = s;
    m_statusImage = m_statusImages[m_status];
}

void UIScreen::setStatus(QString s)
{
    ScreenStatus status = stringToScreenStatus(s.toStdString());
    setStatus(status);
}

bool UIScreen::locked() const
{
    return m_locked;
}

void UIScreen::setLocked(bool value)
{
    m_locked = value;
}

ScreenStatus UIScreen::status() const
{
    return m_status;
}

ErrorCode UIScreen::lastErrorCode() const
{
    return m_lastErrorCode;
}

int UIScreen::version() const
{
    return m_version;
}

void UIScreen::setVersion(int version)
{
    m_version = version;
}

void UIScreen::touch()
{
    m_version++;
}

void UIScreen::setLastErrorCode(const ErrorCode &lastErrorCode)
{
    m_lastErrorCode = lastErrorCode;
}

int UIScreen::id() const
{
    return m_id;
}

void UIScreen::setId(int id)
{
    m_id = id;
}
