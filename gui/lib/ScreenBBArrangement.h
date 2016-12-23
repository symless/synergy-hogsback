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

	bool addScreen(ScreenListModel* screenListModel, Screen& screen);
	bool removeScreen(ScreenListModel* screenListModel, Screen& screen);
	void setViewW(int w);
	void setViewH(int h);
	void adjustModel(ScreenListModel* screenListModel, int index);
	void update(ScreenListModel* screenListModel);
	void printDebugInfo();
	void checkAdjustment(ScreenListModel* screenListModel,
			bool forceCenter = false);
	QList<int> getScreensNextTo(ScreenListModel* screenListModel,
								int index, Direction dir);

private:
	void setScaledViewW(int w);
	void setScaledViewH(int h);
	void calculateNewPos(ScreenListModel* screenListModel, Screen& screen);
	void recalculateBoundingBox(ScreenListModel* screenListModel,
			int ignoreIndex = -1);
	void adjustToCenter(ScreenListModel* screenListModel);
	void snapToOthers(ScreenListModel* screenListModel, int index);
	bool collide(const Screen& src, const Screen& des) const;
	bool collide(const int srcX, const int srcY, const int srcW,
			const int srcH, const int desX, const int desY, const int desW,
			const int desH) const;
	QList<int> collideWithOthers(ScreenListModel* screenListModel, int index) const;
	bool testSnapTo(ScreenListModel* screenListModel, int fromIndex, int toIndex,
			int innerThreshold, int outsideThreshold) const;
	bool testSnapToOnAxis(int from, int to,
			int innerThreshold, int outsideThreshold, bool onAxisX) const;
	QVector2D calculateSnapping(ScreenListModel* screenListModel, int fromIndex,
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
