#ifndef ISCREENARRANGEMENT_H
#define ISCREENARRANGEMENT_H

#include "LibMacro.h"
#include "Direction.h"

#include <QList>

class Screen;
class ScreenModel;

class LIB_SPEC IScreenArrangement
{
public:
	virtual ~IScreenArrangement() { }

	virtual bool addScreen(ScreenModel* screenModel, Screen& screen) = 0;
	virtual bool removeScreen(ScreenModel* screenModel, Screen& screen) = 0;
	virtual void setViewW(int w) = 0;
	virtual void setViewH(int h) = 0;
	virtual void adjustModel(ScreenModel* screenModel, int index) = 0;
	virtual void update(ScreenModel* screenModel) = 0;
	virtual void printDebugInfo() = 0;
	virtual void checkAdjustment(ScreenModel* screenModel,
					bool forceCenter) = 0;
	virtual QList<int> getScreensNextTo(ScreenModel* screenModel,
							int index, Direction dir) = 0;
};

#endif // ISCREENARRANGEMENT_H
