#ifndef SCREENBBARRANGEMENT_H
#define SCREENBBARRANGEMENT_H

#include "LibMacro.h"
#include "IScreenArrangement.h"

#include <QList>
#include <QVector2D>

class LIB_SPEC ScreenBBArrangement : public IScreenArrangement
{
	friend class ScreenBBArrangementTest;
public:
	ScreenBBArrangement();

	bool addScreen(ScreenModel* screenModel, Screen& screen);
	bool removeScreen(ScreenModel* screenModel, Screen& screen);
	void setViewW(int w);
	void setViewH(int h);
	void adjustModel(ScreenModel* screenModel, int index);
	void update(ScreenModel* screenModel);
	void printDebugInfo();
	void checkAdjustment(ScreenModel* screenModel,
			bool forceCenter = false);
	QList<int> getScreensNextTo(ScreenModel* screenModel,
								int index, Direction dir);

private:
	void setScaledViewW(int w);
	void setScaledViewH(int h);
	void calculateNewPos(ScreenModel* screenModel, Screen& screen);
	void recalculateBoundingBox(ScreenModel* screenModel,
			int ignoreIndex = -1);
	void adjustToCenter(ScreenModel* screenModel);
	void snapToOthers(ScreenModel* screenModel, int index);
	bool collide(const Screen& src, const Screen& des) const;
	bool collide(const int srcX, const int srcY, const int srcW,
			const int srcH, const int desX, const int desY, const int desW,
			const int desH) const;
	QList<int> collideWithOthers(ScreenModel* screenModel, int index) const;
	bool testSnapTo(ScreenModel* screenModel, int fromIndex, int toIndex,
			int innerThreshold, int outsideThreshold) const;
	bool testSnapToOnAxis(int from, int to,
			int innerThreshold, int outsideThreshold, bool onAxisX) const;
	QVector2D calculateSnapping(ScreenModel* screenModel, int fromIndex,
			int toIndex) const;
	QVector2D combineAdjustments(QList<QVector2D>& adjustments) const;

private:
	int m_originalViewW;
	int m_originalViewH;
	int m_scaledViewW;
	int m_scaledViewH;
	int m_topLeftX;
	int m_topLeftY;
	int m_bottomRightX;
	int m_bottomRightY;
	float m_scale;
};

#endif // SCREENBBARRANGEMENT_H
