#include "FontManager.h"

#include "LogManager.h"

#include <QFontDatabase>

static QList<QString> kFontList = {
                            ":/Raleway.otf",
                            ":/Museo.otf",
                            ":/AlternateGotNo3D.otf"};

FontManager::FontManager()
{

}

void FontManager::loadAll()
{
    foreach (QString font , kFontList) {
        FontManager::load(font);
    }
}

void FontManager::load(const QString& filename)
{
    QFile file(filename);
    if (!file.exists()) {
        LogManager::warning(QString("no font file: %1").arg(filename.mid(2)));
        return;
    }

    int r = QFontDatabase::addApplicationFont(filename);

    if (r == -1) {
        int dot = filename.lastIndexOf('.');
        QString font = filename.mid(2, dot - 2);
        LogManager::warning(QString("failed to load embeded font: %1").arg(font));
    }
}
