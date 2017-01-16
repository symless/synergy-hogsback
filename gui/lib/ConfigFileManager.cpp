#include "ConfigFileManager.h"

#include "DirectoryManager.h"
#include "IScreenArrangement.h"
#include "LogManager.h"
#include "ScreenListModel.h"
#include "Common.h"

#include <QTextStream>
#include <QFile>
#include <QVector2D>

ConfigFileManager::ConfigFileManager(ScreenListModel* screens,
        IScreenArrangement* arrangementStrategy) :
    m_screens(screens),
    m_arrangementStrategy(arrangementStrategy)
{

}

void ConfigFileManager::writeConfigurationFile(QString path)
{
    QString filename;
    if (path.isEmpty()) {
        DirectoryManager directoryManager;
        filename = directoryManager.configFileDir() + kDefaultConfigFile;
    }
    else {
        filename = path + kDefaultConfigFile;
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);

        // screens section
        stream << "section: screens" << endl;
        for (int i = 0; i < m_screens->getScreenModeSize(); i++) {
            Screen screen = m_screens->getScreen(i);
            stream << "\t" << screen.name() << ':' << endl;
            writeDefaultScreenSettings(stream);
        }
        stream << "end" << endl;

        stream << endl;

        // link section
        writeLinkSection(stream);

        stream << endl;

        //options section
        writeDefaultOptionSettings(stream);

        file.close();

        LogManager::info("write configuration file complete");
    }
    else {
        LogManager::error(QString("fail to open configuration file: %1")
                        .arg(filename));
    }
}


void ConfigFileManager::writeDefaultScreenSettings(QTextStream& stream)
{
    stream << "\t\t" << "halfDuplexCapsLock = false" << endl;
    stream << "\t\t" << "halfDuplexNumLock = false" << endl;
    stream << "\t\t" << "halfDuplexScrollLock = false" << endl;
    stream << "\t\t" << "xtestIsXineramaUnaware = false" << endl;
    stream << "\t\t" << "switchCorners = none" << endl;
    stream << "\t\t" << "switchCornerSize = 0" << endl;
}

void ConfigFileManager::writeDefaultOptionSettings(QTextStream& stream)
{
    stream << "section: options" << endl;
    stream << "\t\t" << "relativeMouseMoves = true" << endl;
    stream << "\t\t" << "screenSaverSync = true" << endl;
    stream << "\t\t" << "win32KeepForeground = false" << endl;
    stream << "\t\t" << "switchCorners = none" << endl;
    stream << "\t\t" << "switchCornerSize = 0" << endl;
    stream << "end" << endl;
}

void ConfigFileManager::writeLinkSection(QTextStream& stream)
{
    stream << "section: links" << endl;

    for (int i = 0; i < m_screens->getScreenModeSize(); i++) {
        const Screen screen = m_screens->getScreen(i);
        stream << "\t" << screen.name() << ':' << endl;

        // check left
        QList<int> leftScreens = m_arrangementStrategy->getScreensNextTo(
                                                            m_screens,
                                                            i, kLeft);
        for (int j = 0; j < leftScreens.count(); j++) {
            const Screen s = m_screens->getScreen(leftScreens[j]);
            writeRelativePosition(stream, screen, s, kLeft);
        }

        // check right
        QList<int> rightScreens = m_arrangementStrategy->getScreensNextTo(
                                                            m_screens,
                                                            i, kRight);
        for (int j = 0; j < rightScreens.count(); j++) {
            const Screen s = m_screens->getScreen(rightScreens[j]);
            writeRelativePosition(stream, screen, s, kRight);
        }

        // check up
        QList<int> upScreens = m_arrangementStrategy->getScreensNextTo(
                                                            m_screens,
                                                            i, kUp);
        for (int j = 0; j < upScreens.count(); j++) {
            const Screen s = m_screens->getScreen(upScreens[j]);
            writeRelativePosition(stream, screen, s, kUp);
        }

        // check down
        QList<int> downScreens = m_arrangementStrategy->getScreensNextTo(
                                                            m_screens,
                                                            i, kDown);
        for (int j = 0; j < downScreens.count(); j++) {
            const Screen s = m_screens->getScreen(downScreens[j]);
            writeRelativePosition(stream, screen, s, kDown);
        }
    }

    stream << "end" << endl;
}

void ConfigFileManager::writeRelativePosition(QTextStream& stream,
                            const Screen& src, const Screen& des,
                            Direction dir)
{
    QString dirStr;
    switch (dir) {
    case kLeft:
        dirStr = "left";
        break;
    case kRight:
        dirStr = "right";
        break;
    case kUp:
        dirStr = "up";
        break;
    case kDown:
        dirStr = "down";
        break;
    }

    QVector2D srcInterval(0, 100);
    QVector2D desInterval(0, 100);

    calculatRelativePercentage(src, des, dir, srcInterval, desInterval);

    stream << "\t\t";
    stream << dirStr;

    if (srcInterval.x() != 0 || srcInterval.y() != 100) {
        stream << "(" << srcInterval.x() << "," << srcInterval.y() << ")";
    }

    stream << " = ";
    stream << des.name();

    if (desInterval.x() != 0 || desInterval.y() != 100) {
        stream << "(" << desInterval.x() << "," << desInterval.y() << ")";
    }

    stream << endl;
}

void ConfigFileManager::calculatRelativePercentage(const Screen& src,
                            const Screen& des, Direction dir,
                            QVector2D& srcInterval, QVector2D& desInterval)
{
    if (dir == kLeft || dir == kRight) {
        int diffY = qAbs(src.posY() - des.posY());
        int overlap = kScreenIconHeight - diffY;
        if (overlap <= 0) return;

        if (src.posY() > des.posY()) {
            srcInterval.setY(overlap * 100 / kScreenIconHeight);
            desInterval.setX(diffY * 100 / kScreenIconHeight);
        }
        else if (src.posY() < des.posY()) {
            srcInterval.setX(diffY * 100 / kScreenIconHeight);
            desInterval.setY(overlap * 100 / kScreenIconHeight);
        }
    }
    else if (dir == kUp || dir == kDown) {
        int diffX = qAbs(src.posX() - des.posX());
        int overlap = kScreenIconWidth - diffX;
        if (overlap <= 0) return;

        if (src.posX() < des.posX()) {
            srcInterval.setX(diffX * 100 / kScreenIconWidth);
            desInterval.setY(overlap * 100 / kScreenIconWidth);
        }
        else if (src.posX() > des.posX()) {
            srcInterval.setY(overlap * 100 / kScreenIconWidth);
            desInterval.setX(diffX * 100 / kScreenIconWidth);
        }
    }
}
