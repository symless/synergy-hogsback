#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QString>

class FontManager
{
public:
    FontManager();

    static void loadAll();
    static void load(const QString& filename);
};

#endif // FONTMANAGER_H
