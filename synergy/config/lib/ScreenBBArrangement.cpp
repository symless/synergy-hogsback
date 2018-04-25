#include "ScreenBBArrangement.h"

#include "UIScreen.h"
#include "ScreenListModel.h"
#include "LogManager.h"
#include <synergy/config/lib/Hostname.h>
#include "Common.h"

#include <QtNetwork>

ScreenBBArrangement::ScreenBBArrangement() :
	m_originalViewW(kDefaultViewWidth),
	m_originalViewH(kDefaultViewHeight),
	m_scaledViewW(kDefaultViewWidth),
	m_scaledViewH(kDefaultViewHeight),
	m_topLeftX(0),
	m_topLeftY(0),
	m_bottomRightX(0),
	m_bottomRightY(0),
	m_scale(1.0f)
{

}

bool ScreenBBArrangement::addScreen(ScreenListModel* screenListModel,
								UIScreen& screen)
{
	calculateNewPos(screenListModel, screen);
	screenListModel->addScreen(screen);
	update(screenListModel);

	LogManager::debug(QString("new added screen pos: %1 %2")
					.arg(screen.posX())
					.arg(screen.posY()));

	return true;
}

bool ScreenBBArrangement::removeScreen(ScreenListModel* screenListModel,
                                int screenId)
{
    screenListModel->removeScreen(screenId);
	update(screenListModel);

	return true;
}

void ScreenBBArrangement::setViewW(int w)
{
	m_originalViewW = w;
}

void ScreenBBArrangement::setViewH(int h)
{
	m_originalViewH = h;
}

void ScreenBBArrangement::adjustModel(ScreenListModel* screenListModel, int index)
{
    // Fixme: this function doesn't handle a case when there is no change
    // in the adjust screen

	// check if we can just use snapping
	bool canSnap = true;
	if (screenListModel->getScreenModeSize() == 1) {
		canSnap = false;
	}

	for (int i = 0; i < screenListModel->getScreenModeSize(); i++) {
		if (i == index) continue;

		bool ok = testSnapTo(screenListModel, index, i,
						kSnappingThreshold, INT_MAX);
		if (!ok) {
			canSnap = false;
			break;
		}
	}

	if (canSnap) {
		int backupX = screenListModel->getScreen(index).posX();
		int backupY = screenListModel->getScreen(index).posY();

		snapToOthers(screenListModel, index);
		// check if overlap exists after snapping
		QList<int> intersectedModels = collideWithOthers(screenListModel, index);

		// no overlap exists, just return
		if (intersectedModels.isEmpty()) {
			update(screenListModel);
			return;
		}
		// revert snapping operation and fall into next adjustment section
		else {
			int offsetX = screenListModel->getScreen(index).posX() - backupX;
			int offsetY = screenListModel->getScreen(index).posY() - backupY;
			screenListModel->moveModel(index, offsetX, offsetY);
		}
	}

	// adjust it according to the whole bounding box
	recalculateBoundingBox(screenListModel, index);
	const UIScreen& original = screenListModel->getScreen(index);
	UIScreen screen("temp");
	screen.setPosX(original.posX());
	screen.setPosY(original.posY());
	calculateNewPos(screenListModel, screen);
	screenListModel->moveModel(index, screen.posX() - original.posX(),
					screen.posY() - original.posY());
	update(screenListModel);
}

void ScreenBBArrangement::update(ScreenListModel* screenListModel)
{
	recalculateBoundingBox(screenListModel);
	checkAdjustment(screenListModel);
}

void ScreenBBArrangement::printDebugInfo()
{
	LogManager::debug(QString("Bounding Box info: %1,%2 -> %3,%4")
					.arg(m_topLeftX).arg(m_topLeftY)
					.arg(m_bottomRightX).arg(m_bottomRightY));
}

void ScreenBBArrangement::calculateNewPos(ScreenListModel* screenListModel,
								UIScreen& screen)
{
	int newPosX = m_scaledViewW / 2 - kScreenIconWidth / 2;
	int newPosY = m_scaledViewH / 2 - kScreenIconHeight / 2;

	if (screen.posX() == -1 && screen.posY() == -1) {
        screenListModel->getScreenPos(Hostname::local(), newPosX, newPosY);
	}
	else {
		newPosX = screen.posX();
		newPosY = screen.posY();
	}

	if (m_topLeftX - m_bottomRightX == 0 || m_topLeftY - m_bottomRightY == 0) {
		screen.setPosX(newPosX);
		screen.setPosY(newPosY);
	}
	else {
		int checkPointX = newPosX + kScreenIconWidth / 2;
		int checkPointY = newPosY + kScreenIconHeight / 2;
		int disToLeft = abs(checkPointX - m_topLeftX)
				* 200 / kScreenIconWidth;
		int disToRight = abs(checkPointX - m_bottomRightX)
				* 200 / kScreenIconWidth;
		int disToTop = abs(checkPointY - m_topLeftY)
				* 200 / kScreenIconHeight;
		int disToBottom = abs(checkPointY - m_bottomRightY)
				* 200 / kScreenIconHeight;

		int minDisX = disToLeft < disToRight ? disToLeft : disToRight;
		int minDisY = disToTop < disToBottom ? disToTop : disToBottom;
		int maxDisX = disToLeft > disToRight ? disToLeft : disToRight;
		int maxDisY = disToTop > disToBottom ? disToTop : disToBottom;

		if (minDisX < minDisY ||
			(minDisX == minDisY && maxDisY <= maxDisX)) {
			// align to the left
			if (disToLeft < disToRight) {
				screen.setPosX(m_topLeftX - kScreenIconWidth);
				screen.setPosY(newPosY);
			}
			// align to the right
			else {
				screen.setPosX(m_bottomRightX);
				screen.setPosY(newPosY);
			}
		}
		else if (minDisX > minDisY ||
				 (minDisX == minDisY && maxDisY > maxDisX)) {
			// align to the top
			if (disToTop < disToBottom) {
				screen.setPosX(newPosX);
				screen.setPosY(m_topLeftY - kScreenIconHeight);
			}
			// align to the bottom
			else {
				screen.setPosX(newPosX);
				screen.setPosY(m_bottomRightY);
			}
		}
	}
}

void ScreenBBArrangement::checkAdjustment(ScreenListModel* screenListModel,
								bool forceCenter)
{
	int adjustAll = false;
	int boundingBoxWidth = m_bottomRightX - m_topLeftX;
	int boundingBoxHeight = m_bottomRightY - m_topLeftY;

	// outside of the view
	bool wasOutside = false;
	if (m_topLeftX < 0 ||
		m_topLeftY < 0 ||
		m_bottomRightX > m_scaledViewW ||
		m_bottomRightY > m_scaledViewH) {
			adjustAll = true;
			wasOutside = true;
	}

	// the view is too big for showing the screens
	if (boundingBoxWidth * kScaleThreshold < m_scaledViewW ||
		boundingBoxHeight * kScaleThreshold < m_scaledViewH) {
		adjustAll = true;
	}

	if (adjustAll || forceCenter) {
		float timesOnX = boundingBoxWidth / (float)m_originalViewW;
		float timesOnY = boundingBoxHeight / (float)m_originalViewH;

		float maxTimes = timesOnX > timesOnY ? timesOnX : timesOnY;
		float oldScale = m_scale;
		m_scale = 1 / maxTimes;

		if (m_scale > 1.0f) {
			m_scale = 1.0f;
			maxTimes = 1;
		}

		screenListModel->setScale(m_scale);
		setScaledViewW(m_originalViewW * maxTimes);
		setScaledViewH(m_originalViewH * maxTimes);

		if (oldScale > m_scale ||
			(oldScale < 1.0f && m_scale == 1.0f) ||
			wasOutside ||
			forceCenter) {

			adjustToCenter(screenListModel);
		}
	}
}

QList<int> ScreenBBArrangement::getScreensNextTo(ScreenListModel* screenListModel,
									int index, Direction dir)
{
	QList<int> result;
	UIScreen screen = screenListModel->getScreen(index);
	int srcX = screen.posX();
	int srcY = screen.posY();
	int srcW = kScreenIconWidth;
	int srcH = kScreenIconHeight;

	// initialize a dummy rectangle that represends the testing area
	switch(dir) {
	case kLeft: {
		srcX = 0;
		srcW = screen.posX();
		break;
	}
	case kRight: {
		srcX = srcX + kScreenIconWidth;
		srcW = m_scaledViewW - srcX;
		break;
	}
	case kUp: {
		srcY = 0;
		srcH = screen.posY();
		break;
	}
	case kDown: {
		srcY = srcY + kScreenIconHeight;
		srcH = m_scaledViewH - srcY;
		break;
	}
	default: {
		LogManager::warning(QString("unsupport direction is used"));
		return result;
	}
	}

	for (int i = 0; i < screenListModel->getScreenModeSize(); i++) {
		if (i == index) continue;
		UIScreen des = screenListModel->getScreen(i);
		if (collide(srcX, srcY, srcW, srcH, des.posX(), des.posY(),
				kScreenIconWidth, kScreenIconHeight)) {
			result.append(i);
		}
	}

	return result;
}

void ScreenBBArrangement::setScaledViewW(int w)
{
	m_scaledViewW = w;
}

void ScreenBBArrangement::setScaledViewH(int h)
{
	m_scaledViewH = h;
}

void ScreenBBArrangement::recalculateBoundingBox(ScreenListModel* screenListModel,
								int ignoreIndex)
{
	m_topLeftX = INT_MAX;
	m_topLeftY = INT_MAX;
	m_bottomRightX = INT_MIN;
	m_bottomRightY = INT_MIN;

	for (int i = 0; i < screenListModel->getScreenModeSize(); i++) {
		if (i == ignoreIndex) continue;

		const UIScreen& screen = screenListModel->getScreen(i);
		int posX = screen.posX();
		int posY = screen.posY();
		m_topLeftX = m_topLeftX < posX ? m_topLeftX : posX;
		m_topLeftY = m_topLeftY < posY ? m_topLeftY : posY;
		m_bottomRightX = m_bottomRightX > posX + kScreenIconWidth ?
						m_bottomRightX : posX + kScreenIconWidth;
		m_bottomRightY = m_bottomRightY > posY + kScreenIconHeight ?
						m_bottomRightY : posY + kScreenIconHeight;
	}

	if (m_topLeftX == INT_MAX) m_topLeftX = 0;
	if (m_topLeftY == INT_MAX) m_topLeftY = 0;
	if (m_bottomRightX == INT_MIN) m_bottomRightX = 0;
	if (m_bottomRightY == INT_MIN) m_bottomRightY = 0;
}

void ScreenBBArrangement::adjustToCenter(ScreenListModel* screenListModel)
{
	int viewCenterX = m_scaledViewW / 2;
	int viewCenterY = m_scaledViewH / 2;
	int boundingboxCenterX = (m_bottomRightX + m_topLeftX) / 2;
	int boundingboxCenterY = (m_bottomRightY + m_topLeftY) / 2;

	int disX = viewCenterX - boundingboxCenterX;
	int disY = viewCenterY - boundingboxCenterY;

	m_topLeftX += disX;
	m_topLeftY += disY;
	m_bottomRightX += disX;
	m_bottomRightY += disY;

	// make sure bounding box are still in the view
	if (m_topLeftX < 0) {
		disX -= m_topLeftX;
		m_topLeftX = 0;
	}
	if (m_topLeftY < 0) {
		disY -= m_topLeftY;
		m_topLeftY = 0;
	}
	if (m_bottomRightX > m_scaledViewW) {
		disX -= (m_bottomRightX - m_scaledViewW);
		m_bottomRightX = m_scaledViewW;
	}
	if (m_bottomRightY > m_scaledViewH) {
		disY -= (m_bottomRightY - m_scaledViewH);
		m_bottomRightY = m_scaledViewH;
	}

	screenListModel->adjustAll(disX, disY);
}

void ScreenBBArrangement::snapToOthers(ScreenListModel* screenListModel, int index)
{
	QList<QVector2D> adjustments;
	for (int i = 0; i < screenListModel->getScreenModeSize(); i++) {
		if (i == index) continue;
		adjustments.append(calculateSnapping(screenListModel, index, i));
	}

	QVector2D finalAdjustment = combineAdjustments(adjustments);
	screenListModel->moveModel(index,
					finalAdjustment.x(), finalAdjustment.y());
}

bool ScreenBBArrangement::collide(const UIScreen& src, const UIScreen& des) const
{
	bool result = false;

	if (collide(src.posX(), src.posY(), kScreenIconWidth, kScreenIconHeight,
			des.posX(), des.posY(), kScreenIconWidth, kScreenIconHeight)) {
		result = true;
	}

	return result;
}

bool ScreenBBArrangement::collide(const int srcX, const int srcY,
							const int srcW, const int srcH, const int desX,
							const int desY, const int desW, const int desH)
							const
{
	bool result = false;
	if (srcX < desX + desW &&
		srcY < desY + desH &&
		srcX + srcW > desX &&
		srcY + srcH > desY) {
		result = true;
	}

	return result;
}

QList<int> ScreenBBArrangement::collideWithOthers(ScreenListModel* screenListModel,
									int index) const
{
	QList<int> result;
	const UIScreen screenSrc = screenListModel->getScreen(index);
	for (int i = 0; i < screenListModel->getScreenModeSize(); i++) {
		if (i == index) continue;

		const UIScreen& screenDes = screenListModel->getScreen(i);
		if (collide(screenSrc, screenDes)) {
			result.append(i);
		}
	}

	return result;
}

bool ScreenBBArrangement::testSnapTo(ScreenListModel* screenListModel,
							int fromIndex, int toIndex,
							int innerThreshold, int outsideThreshold) const
{
	if (innerThreshold <= 0 || outsideThreshold <=0) return false;

	int fromX = screenListModel->getScreen(fromIndex).posX();
	int fromY = screenListModel->getScreen(fromIndex).posY();
	int toX = screenListModel->getScreen(toIndex).posX();
	int toY = screenListModel->getScreen(toIndex).posY();

	bool canSnapOnX = testSnapToOnAxis(fromX, toX,
						innerThreshold, outsideThreshold, true);
	bool canSnapOnY = testSnapToOnAxis(fromY, toY,
						innerThreshold, outsideThreshold, false);

	bool result = true;
	if (!canSnapOnX && !canSnapOnY) {
		result = false;
	}

	return result;
}

bool ScreenBBArrangement::testSnapToOnAxis(int from, int to,
							int innerThreshold, int outsideThreshold,
							bool onAxisX) const
{
	// calculate 4 marks the indicates the valid interval in which
	// snapping is allowed
	int screenIconSize = onAxisX ? kScreenIconWidth : kScreenIconHeight;

	int intervalMark1 =  0;
	if (INT_MIN + outsideThreshold + screenIconSize > to) {
		intervalMark1 = INT_MIN;
	}
	else {
		intervalMark1 = to - outsideThreshold - screenIconSize;
	}

	int intervalMark2 = to + innerThreshold - screenIconSize;
	int intervalMark3 = to + screenIconSize - innerThreshold;

	int intervalMark4 =  0;
	if (to + screenIconSize > INT_MAX - outsideThreshold) {
		intervalMark4 = INT_MAX;
	}
	else {
		intervalMark4 = to + screenIconSize + outsideThreshold;
	}

	bool canSnapOnAxis = true;
	if (from < intervalMark1 ||
		(from > intervalMark2 && from < intervalMark3) ||
		from > intervalMark4) {
		canSnapOnAxis = false;
	}

	return canSnapOnAxis;

}

QVector2D ScreenBBArrangement::calculateSnapping(ScreenListModel* screenListModel,
									int fromIndex, int toIndex) const
{
	int fromX = screenListModel->getScreen(fromIndex).posX();
	int fromY = screenListModel->getScreen(fromIndex).posY();
	int toX = screenListModel->getScreen(toIndex).posX();
	int toY = screenListModel->getScreen(toIndex).posY();
	int adjustmentX = 0;
	int adjustmentY = 0;

	if (fromX < toX) {
		adjustmentX = toX - kScreenIconWidth - fromX;
	}
	else if (fromX > toX) {
		adjustmentX = toX + kScreenIconWidth - fromX;
	}

	if (fromY < toY) {
		adjustmentY = toY - kScreenIconHeight - fromY;
	}
	else if (fromY > toY) {
		adjustmentY = toY + kScreenIconHeight - fromY;
	}

	if (adjustmentX == 0 || adjustmentY == 0) {
		return QVector2D(adjustmentX, adjustmentY);
	}
	else {
		int tempX = qAbs(adjustmentX);
		int tempY = qAbs(adjustmentY);

		if (tempX < tempY) {
			adjustmentY = 0;
		}
		else if (tempX > tempY) {
			adjustmentX = 0;
		}
	}

	return QVector2D(adjustmentX, adjustmentY);
}

QVector2D ScreenBBArrangement::combineAdjustments(
									QList<QVector2D>& adjustments) const
{
	if (adjustments.isEmpty()) {
		return QVector2D(0, 0);
	}

	int minAdjustmentX = INT_MAX;
	int minAdjustmentY = INT_MAX;

	for (int i = 0; i < adjustments.count(); i++) {
		if (qAbs(minAdjustmentX) > qAbs(adjustments[i].x()) &&
			qAbs(adjustments[i].x()) != 0) {
			minAdjustmentX = adjustments[i].x();
		}

		if (qAbs(minAdjustmentY) > qAbs(adjustments[i].y()) &&
			qAbs(adjustments[i].y()) != 0) {
			minAdjustmentY = adjustments[i].y();
		}
	}

	// only adjust on one axis
	int tempX = qAbs(minAdjustmentX);
	int tempY = qAbs(minAdjustmentY);

	if (tempX > kSnappingThreshold || tempY > kSnappingThreshold) {
		if (tempX < tempY) {
			minAdjustmentY = 0;
		}
		else if (tempX > tempY) {
			minAdjustmentX = 0;
		}
	}

    if (minAdjustmentX == INT_MAX) {
        minAdjustmentX = 0;
    }

    if (minAdjustmentY == INT_MAX) {
        minAdjustmentY = 0;
    }
	return QVector2D(minAdjustmentX, minAdjustmentY);
}
