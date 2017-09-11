#ifndef ISCREENARRANGEMENT_H
#define ISCREENARRANGEMENT_H

#include "LibMacro.h"
#include "Direction.h"

#include <QList>

class UIScreen;
class ScreenListModel;

class LIB_SPEC IScreenArrangement
{
public:
	virtual ~IScreenArrangement() { }

	virtual bool addScreen(ScreenListModel* screenModel, UIScreen& screen) = 0;
	virtual bool removeScreen(ScreenListModel* screenModel, UIScreen& screen) = 0;
	virtual void setViewW(int w) = 0;
	virtual void setViewH(int h) = 0;
	virtual void adjustModel(ScreenListModel* screenModel, int index) = 0;
	virtual void update(ScreenListModel* screenModel) = 0;
	virtual void printDebugInfo() = 0;
	virtual void checkAdjustment(ScreenListModel* screenModel,
					bool forceCenter) = 0;
	virtual QList<int> getScreensNextTo(ScreenListModel* screenModel,
							int index, Direction dir) = 0;
};

#endif // ISCREENARRANGEMENT_H
