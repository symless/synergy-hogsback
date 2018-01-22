#include "UIScreen.h"
#include "LogManager.h"
#include "Common.h"

QHash<ScreenStatus, QString> UIScreen::m_statusImages = {
    {ScreenStatus::kConnected, "qrc:/res/image/screen-active.png"},
    {ScreenStatus::kConnecting, "qrc:/res/image/screen-inactive.png"},
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
    m_errorCode(ScreenError::kNone),
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
UIScreen::helpLink() const
{
    // TODO: reimplement this
    return  "help";
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

ScreenError UIScreen::errorCode() const
{
    return m_errorCode;
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

QString UIScreen::errorMessage() const
{
    return m_errorMessage;
}

void UIScreen::setErrorMessage(const QString &errorMessage)
{
    m_errorMessage = errorMessage;
}

void UIScreen::setErrorCode(const ScreenError &errorCode)
{
    m_errorCode = errorCode;
}

int UIScreen::id() const
{
    return m_id;
}

void UIScreen::setId(int id)
{
    m_id = id;
}
