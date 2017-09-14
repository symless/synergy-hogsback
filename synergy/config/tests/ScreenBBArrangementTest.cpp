#include "ScreenBBArrangementTest.h"

#include "MockScreenModel.h"
#include "ScreenBBArrangement.h"
#include "Common.h"

ScreenBBArrangementTest::ScreenBBArrangementTest(QObject* parent) : QObject(parent)
{

}

void ScreenBBArrangementTest::calculateNewPos_firstNewScreen_addToTheCenter()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    UIScreen screen("mock");
    arrangement.calculateNewPos(&screenListModel, screen);

    QCOMPARE(screen.posX(), kDefaultViewWidth / 2 - kScreenIconWidth / 2);
    QCOMPARE(screen.posY(), kDefaultViewHeight / 2 - kScreenIconHeight / 2);
}

void ScreenBBArrangementTest::calculateNewPos_firstExistScreen_noChangeInScreen()
{
    // this scenario should never happen
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    UIScreen screen("mock");
    screen.setPosX(0);
    screen.setPosY(0);
    arrangement.calculateNewPos(&screenListModel, screen);

    QCOMPARE(screen.posX(), 0);
    QCOMPARE(screen.posY(), 0);
}

void ScreenBBArrangementTest::calculateNewPos_newScreen_alignToLeft()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = kDefaultViewWidth / 2 - kScreenIconWidth / 2;
    arrangement.m_topLeftY = kDefaultViewHeight / 2 - kScreenIconHeight / 2;
    arrangement.m_bottomRightX = kDefaultViewWidth / 2 +
            kScreenIconWidth / 2 + kScreenIconWidth;
    arrangement.m_bottomRightY = kDefaultViewHeight / 2 + kScreenIconHeight / 2;
    UIScreen screen("mock");
    arrangement.calculateNewPos(&screenListModel, screen);

    QCOMPARE(screen.posX(), kDefaultViewWidth / 2 - kScreenIconWidth / 2 -
        kScreenIconWidth);
    QCOMPARE(screen.posY(), kDefaultViewHeight / 2 - kScreenIconHeight / 2);
}

void ScreenBBArrangementTest::calculateNewPos_newScreen_alignToRight()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = kDefaultViewWidth / 2 - kScreenIconWidth / 2;
    arrangement.m_topLeftY = kDefaultViewHeight / 2 - kScreenIconHeight / 2;
    arrangement.m_bottomRightX = kDefaultViewWidth / 2 + kScreenIconWidth / 2;
    arrangement.m_bottomRightY = kDefaultViewHeight / 2 + kScreenIconHeight / 2;
    UIScreen screen("mock");
    arrangement.calculateNewPos(&screenListModel, screen);

    QCOMPARE(screen.posX(), kDefaultViewWidth / 2 + kScreenIconWidth / 2);
    QCOMPARE(screen.posY(), kDefaultViewHeight / 2 - kScreenIconHeight / 2);
}

void ScreenBBArrangementTest::calculateNewPos_newScreen_alignToUp()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = kDefaultViewWidth / 2 - kScreenIconWidth / 2 -
            kScreenIconWidth;
    arrangement.m_topLeftY = kDefaultViewHeight / 2 - kScreenIconHeight / 2;
    arrangement.m_bottomRightX = kDefaultViewWidth / 2 +
            kScreenIconWidth / 2 + kScreenIconWidth;
    arrangement.m_bottomRightY = kDefaultViewHeight / 2 +
            kScreenIconHeight / 2 + kDefaultViewHeight;
    UIScreen screen("mock");
    arrangement.calculateNewPos(&screenListModel, screen);

    QCOMPARE(screen.posX(), kDefaultViewWidth / 2 - kScreenIconWidth / 2);
    QCOMPARE(screen.posY(), kDefaultViewHeight / 2 - kScreenIconHeight / 2
        - kScreenIconHeight);
}

void ScreenBBArrangementTest::calculateNewPos_newScreen_alignToDown()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = kDefaultViewWidth / 2 - kScreenIconWidth / 2 -
            kScreenIconWidth;
    arrangement.m_topLeftY = kDefaultViewHeight / 2 - kScreenIconHeight / 2;
    arrangement.m_bottomRightX = kDefaultViewWidth / 2 +
            kScreenIconWidth / 2 + kScreenIconWidth;
    arrangement.m_bottomRightY = kDefaultViewHeight / 2 + kScreenIconHeight / 2;
    UIScreen screen("mock");
    arrangement.calculateNewPos(&screenListModel, screen);

    QCOMPARE(screen.posX(), kDefaultViewWidth / 2 - kScreenIconWidth / 2);
    QCOMPARE(screen.posY(), kDefaultViewHeight / 2 + kScreenIconHeight / 2);
}

void ScreenBBArrangementTest::checkAdjustment_outsideOfView_noScale()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = -1;
    arrangement.m_topLeftY = -1;
    arrangement.m_bottomRightX = kScreenIconWidth - 1;
    arrangement.m_bottomRightY = kScreenIconHeight - 1;
    arrangement.checkAdjustment(&screenListModel);

    QCOMPARE(arrangement.m_scale, 1.0f);
}

void ScreenBBArrangementTest::checkAdjustment_outsideOfView_scale()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = -kDefaultViewWidth / 2;
    arrangement.m_topLeftY = 0;
    arrangement.m_bottomRightX = kDefaultViewWidth / 2 * 3;
    arrangement.m_bottomRightY = kDefaultViewHeight;
    arrangement.checkAdjustment(&screenListModel);

    QCOMPARE(arrangement.m_scale, 0.5f);
    QCOMPARE(arrangement.m_scaledViewW, kDefaultViewWidth * 2);
    QCOMPARE(arrangement.m_scaledViewH, kDefaultViewHeight * 2);
}

void ScreenBBArrangementTest::checkAdjustment_viewTooBig_scale()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_scaledViewW = kDefaultViewWidth * 4;
    arrangement.m_scaledViewH = kDefaultViewHeight * 4;
    arrangement.m_scale = 0.25f;
    arrangement.m_topLeftX = 0;
    arrangement.m_topLeftY = 0;
    arrangement.m_bottomRightX = kDefaultViewWidth * 2;
    arrangement.m_bottomRightY = kDefaultViewHeight * 2;
    arrangement.checkAdjustment(&screenListModel);

    QCOMPARE(arrangement.m_scale, 0.5f);
    QCOMPARE(arrangement.m_scaledViewW, kDefaultViewWidth * 2);
    QCOMPARE(arrangement.m_scaledViewH, kDefaultViewHeight * 2);
}

void ScreenBBArrangementTest::checkAdjustment_overScale_noScale()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.m_scaledViewW = kDefaultViewWidth;
    arrangement.m_scaledViewH = kDefaultViewHeight;
    arrangement.m_scale = 1.0f;
    arrangement.m_topLeftX = 0;
    arrangement.m_topLeftY = 0;
    arrangement.m_bottomRightX = kScreenIconWidth;
    arrangement.m_bottomRightY = kScreenIconHeight;
    arrangement.checkAdjustment(&screenListModel);

    QCOMPARE(arrangement.m_scale, 1.0f);
    QCOMPARE(arrangement.m_scaledViewW, kDefaultViewWidth);
    QCOMPARE(arrangement.m_scaledViewH, kDefaultViewHeight);
}

void ScreenBBArrangementTest::getScreensNextTo_nonexistOnLeft_emptyList()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kLeft);

    QCOMPARE(r.count(), 0);
}

void ScreenBBArrangementTest::getScreensNextTo_oneExistOnLeft_listWithOneIndex()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(kScreenIconWidth);
    screen1.setPosY(kScreenIconHeight - 1);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kLeft);

    QCOMPARE(r.count(), 1);
    QCOMPARE(r[0], 1);
}

void ScreenBBArrangementTest::getScreensNextTo_nonexistOnRight_emptyList()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kRight);

    QCOMPARE(r.count(), 0);
}

void ScreenBBArrangementTest::getScreensNextTo_twoExistOnRight_listWithTwoIndices()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight - 1);
    UIScreen screen3("mock3");
    screen3.setPosX(kScreenIconWidth * 2);
    screen3.setPosY(kScreenIconHeight - 1);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    screenListModel.addScreen(screen3);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kRight);

    QCOMPARE(r.count(), 2);
    QCOMPARE(r[0], 1);
    QCOMPARE(r[1], 2);
}

void ScreenBBArrangementTest::getScreensNextTo_nonexistOnTop_emptyList()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kUp);

    QCOMPARE(r.count(), 0);
}

void ScreenBBArrangementTest::getScreensNextTo_oneExistOnTop_listWithOneIndex()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(kScreenIconWidth - 1);
    screen1.setPosY(kScreenIconHeight);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kUp);

    QCOMPARE(r.count(), 1);
    QCOMPARE(r[0], 1);
}

void ScreenBBArrangementTest::getScreensNextTo_nonexistOnBottom_emptyList()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kDown);

    QCOMPARE(r.count(), 0);
}

void ScreenBBArrangementTest::getScreensNextTo_twoExistOnBottom_listWithTwoIndices()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(kScreenIconHeight);
    UIScreen screen3("mock3");
    screen3.setPosX(kScreenIconWidth - 1);
    screen3.setPosY(kScreenIconHeight * 2);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    screenListModel.addScreen(screen3);
    ScreenBBArrangement arrangement;
    QList<int> r = arrangement.getScreensNextTo(&screenListModel, 0, kDown);

    QCOMPARE(r.count(), 2);
    QCOMPARE(r[0], 1);
    QCOMPARE(r[1], 2);
}

void ScreenBBArrangementTest::recalculateBoundingBox_oneScreen_newBoundingBox()
{
    MockScreenModel screenListModel;
    UIScreen screen("mock");
    screen.setPosX(0);
    screen.setPosY(0);
    screenListModel.addScreen(screen);
    ScreenBBArrangement arrangement;
    arrangement.recalculateBoundingBox(&screenListModel);

    QCOMPARE(arrangement.m_topLeftX, 0);
    QCOMPARE(arrangement.m_topLeftY, 0);
    QCOMPARE(arrangement.m_bottomRightX, kScreenIconWidth);
    QCOMPARE(arrangement.m_bottomRightY, kScreenIconHeight);
}

void ScreenBBArrangementTest::recalculateBoundingBox_twoScreen_newBoundingBox()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    arrangement.recalculateBoundingBox(&screenListModel);

    QCOMPARE(arrangement.m_topLeftX, 0);
    QCOMPARE(arrangement.m_topLeftY, 0);
    QCOMPARE(arrangement.m_bottomRightX, kScreenIconWidth * 2);
    QCOMPARE(arrangement.m_bottomRightY, kScreenIconHeight * 2);
}

void ScreenBBArrangementTest::recalculateBoundingBox_emptyModel_zeroBoundingBox()
{
    MockScreenModel screenListModel;
    ScreenBBArrangement arrangement;
    arrangement.recalculateBoundingBox(&screenListModel);

    QCOMPARE(arrangement.m_topLeftX, 0);
    QCOMPARE(arrangement.m_topLeftY, 0);
    QCOMPARE(arrangement.m_bottomRightX, 0);
    QCOMPARE(arrangement.m_bottomRightY, 0);
}

void ScreenBBArrangementTest::recalculateBoundingBox_skipOne_zeroBoundingBox()
{
    MockScreenModel screenListModel;
    UIScreen screen("mock");
    screen.setPosX(0);
    screen.setPosY(0);
    screenListModel.addScreen(screen);
    ScreenBBArrangement arrangement;
    arrangement.recalculateBoundingBox(&screenListModel, 0);

    QCOMPARE(arrangement.m_topLeftX, 0);
    QCOMPARE(arrangement.m_topLeftY, 0);
    QCOMPARE(arrangement.m_bottomRightX, 0);
    QCOMPARE(arrangement.m_bottomRightY, 0);
}

void ScreenBBArrangementTest::recalculateBoundingBox_skipOneInTwo_zeroBoundingBox()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(kScreenIconHeight);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;
    arrangement.recalculateBoundingBox(&screenListModel, 0);

    QCOMPARE(arrangement.m_topLeftX, kScreenIconWidth);
    QCOMPARE(arrangement.m_topLeftY, kScreenIconHeight);
    QCOMPARE(arrangement.m_bottomRightX, kScreenIconWidth * 2);
    QCOMPARE(arrangement.m_bottomRightY, kScreenIconHeight * 2);
}

void ScreenBBArrangementTest::adjustToCenter_oneScreen_moveScreenToCenter()
{
    MockScreenModel screenListModel;
    UIScreen screen("mock");
    screen.setPosX(0);
    screen.setPosY(0);
    screenListModel.addScreen(screen);
    ScreenBBArrangement arrangement;
    arrangement.m_topLeftX = 0;
    arrangement.m_topLeftY = 0;
    arrangement.m_bottomRightX = kScreenIconWidth;
    arrangement.m_bottomRightY = kScreenIconHeight;
    arrangement.adjustToCenter(&screenListModel);

    QCOMPARE(screenListModel.m_disX,
        (kDefaultViewWidth / 2) - (kScreenIconWidth / 2));
    QCOMPARE(screenListModel.m_disY,
        (kDefaultViewHeight / 2) - (kScreenIconHeight / 2));
}

void ScreenBBArrangementTest::collide_overlap_returnTrue()
{
    const int srcX = 0;
    const int srcY = 0;
    const int srcW = kScreenIconWidth;
    const int srcH = kScreenIconHeight;
    const int desX = kScreenIconWidth - 1;
    const int desY = kScreenIconHeight - 1;
    const int desW = kScreenIconWidth;
    const int desH = kScreenIconHeight;

    ScreenBBArrangement arrangement;
    bool r =arrangement.collide(srcX, srcY, srcW, srcH,
                            desX, desY, desW, desH);
    QCOMPARE(r, true);
}

void ScreenBBArrangementTest::collide_noOverlap_returnFalse()
{
    const int srcX = 0;
    const int srcY = 0;
    const int srcW = kScreenIconWidth;
    const int srcH = kScreenIconHeight;
    const int desX = kScreenIconWidth;
    const int desY = 0;
    const int desW = kScreenIconWidth;
    const int desH = kScreenIconHeight;

    ScreenBBArrangement arrangement;
    bool r =arrangement.collide(srcX, srcY, srcW, srcH,
                            desX, desY, desW, desH);
    QCOMPARE(r, false);
}

void ScreenBBArrangementTest::collideWithOthers_overlap_returOverlapList()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth - 1);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QList<int> r = arrangement.collideWithOthers(&screenListModel, 0);

    QCOMPARE(r.count(), 1);
    QCOMPARE(r[0], 1);
}

void ScreenBBArrangementTest::collideWithOthers_noOverlap_emptyList()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QList<int> r = arrangement.collideWithOthers(&screenListModel, 0);

    QCOMPARE(r.count(), 0);
}

void ScreenBBArrangementTest::testSnapToOnAxis_withinLeftInnerThreshold_returnTrue()
{
    int from = 0;
    int to = kScreenIconWidth - kSnappingThreshold + 1;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to,
                            kSnappingThreshold, kSnappingThreshold, true);

    QCOMPARE(r, true);
}

void ScreenBBArrangementTest::testSnapToOnAxis_withinLeftOutsideThreshold_returnTrue()
{
    int from = 0;
    int to = kScreenIconWidth + kSnappingThreshold - 1;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to,
                            kSnappingThreshold, kSnappingThreshold, true);

    QCOMPARE(r, true);
}

void ScreenBBArrangementTest::testSnapToOnAxis_withinRightInnerThreshold_returnTrue()
{
    int to = kScreenIconWidth;
    int from = to + kScreenIconWidth - kSnappingThreshold + 1;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to,
                            kSnappingThreshold, kSnappingThreshold, true);

    QCOMPARE(r, true);
}

void ScreenBBArrangementTest::testSnapToOnAxis_withinRightOutsideThreshold_returnTrue()
{
    int to = kScreenIconWidth;
    int from = to + kScreenIconWidth + kSnappingThreshold - 1;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to, kSnappingThreshold,
                            kSnappingThreshold, true);

    QCOMPARE(r, true);
}

void ScreenBBArrangementTest::testSnapToOnAxis_outsideLeftOutsideThreshold_returnfalse()
{
    int from = 10 - kSnappingThreshold - 1;
    int to = kScreenIconWidth + 10;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to,
                            kSnappingThreshold, kSnappingThreshold, true);

    QCOMPARE(r, false);
}

void ScreenBBArrangementTest::testSnapToOnAxis_withinTwoInnerThresholds_returnfalse()
{
    int from = kSnappingThreshold + 1;
    int to = kScreenIconWidth;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to,
                            kSnappingThreshold, kSnappingThreshold, true);

    QCOMPARE(r, false);
}

void ScreenBBArrangementTest::testSnapToOnAxis_outsideRightOutsideThreshold_returnfalse()
{
    int from = kScreenIconWidth + kSnappingThreshold + 1;
    int to = kScreenIconWidth;
    ScreenBBArrangement arrangement;
    bool r =arrangement.testSnapToOnAxis(from, to, kSnappingThreshold,
                kSnappingThreshold, true);

    QCOMPARE(r, false);
}

void ScreenBBArrangementTest::calculateSnapping_onlyCloseToRight_snapToRight()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(kScreenIconWidth + 3);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), -3.0f);
    QCOMPARE(r.y(), 0.0);
}

void ScreenBBArrangementTest::calculateSnapping_closeToRightMoreThanTop_snapToRight()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(kScreenIconWidth + 3);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(5);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), -3.0f);
    QCOMPARE(r.y(), 0.0);
}

void ScreenBBArrangementTest::calculateSnapping_onlyCloseToLeft_snapToLeft()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth + 3);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), 3.0f);
    QCOMPARE(r.y(), 0.0);
}

void ScreenBBArrangementTest::calculateSnapping_closeToLeftMoreThanBottom_snapToLeft()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(kScreenIconWidth + 3);
    screen2.setPosY(5);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), 3.0f);
    QCOMPARE(r.y(), 0.0);
}

void ScreenBBArrangementTest::calculateSnapping_onlyCloseToTop_snapToTop()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(kScreenIconHeight + 3);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), 3.0);
}

void ScreenBBArrangementTest::calculateSnapping_closeToTopMoreThanLeft_snapToTop()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(0);
    UIScreen screen2("mock2");
    screen2.setPosX(5);
    screen2.setPosY(kScreenIconHeight + 3);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), 3.0);
}

void ScreenBBArrangementTest::calculateSnapping_onlyCloseToBottom_snapToBottom()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(0);
    screen1.setPosY(kScreenIconHeight + 3);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), -3.0);
}

void ScreenBBArrangementTest::calculateSnapping_closeToBottomMoreThanRight_snapToBottom()
{
    MockScreenModel screenListModel;
    UIScreen screen1("mock1");
    screen1.setPosX(5);
    screen1.setPosY(kScreenIconHeight + 3);
    UIScreen screen2("mock2");
    screen2.setPosX(0);
    screen2.setPosY(0);
    screenListModel.addScreen(screen1);
    screenListModel.addScreen(screen2);
    ScreenBBArrangement arrangement;

    QVector2D r = arrangement.calculateSnapping(&screenListModel, 0, 1);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), -3.0);
}

void ScreenBBArrangementTest::combineAdjustments_noMoveOnY_returnMinimumMoveOnX()
{
    QList<QVector2D> adjustments;
    QVector2D adj1(1, 0);
    QVector2D adj2(2, 0);
    QVector2D adj3(3, 0);
    adjustments.append(adj1);
    adjustments.append(adj2);
    adjustments.append(adj3);

    ScreenBBArrangement arrangement;
    QVector2D r =arrangement.combineAdjustments(adjustments);

    QCOMPARE(r.x(), 1.0f);
    QCOMPARE(r.y(), 0.0);
}

void ScreenBBArrangementTest::combineAdjustments_noMoveOnX_returnMinimumMoveOnY()
{
    QList<QVector2D> adjustments;
    QVector2D adj1(0, 3);
    QVector2D adj2(0, 2);
    QVector2D adj3(0, 1);
    adjustments.append(adj1);
    adjustments.append(adj2);
    adjustments.append(adj3);

    ScreenBBArrangement arrangement;
    QVector2D r =arrangement.combineAdjustments(adjustments);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), 1.0);
}

void ScreenBBArrangementTest::combineAdjustments_onlyYMovementExceedSnapThreshold_returnMinimumMoveOnX()
{
    QList<QVector2D> adjustments;
    QVector2D adj1(1, 10);
    QVector2D adj2(2, 9);
    QVector2D adj3(3, 8);
    adjustments.append(adj1);
    adjustments.append(adj2);
    adjustments.append(adj3);

    ScreenBBArrangement arrangement;
    QVector2D r =arrangement.combineAdjustments(adjustments);

    QCOMPARE(r.x(), 1.0f);
    QCOMPARE(r.y(), 0.0);
}

void ScreenBBArrangementTest::combineAdjustments_onlyXMovementExceedSnapThreshold_returnMinimumMoveOnY()
{
    QList<QVector2D> adjustments;
    QVector2D adj1(8, 3);
    QVector2D adj2(9, 2);
    QVector2D adj3(10, 1);
    adjustments.append(adj1);
    adjustments.append(adj2);
    adjustments.append(adj3);

    ScreenBBArrangement arrangement;
    QVector2D r =arrangement.combineAdjustments(adjustments);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), 1.0f);
}

void ScreenBBArrangementTest::combineAdjustments_allWithinSnapThreshold_returnBothAdjustments()
{
    QList<QVector2D> adjustments;
    QVector2D adj1(2, 3);
    QVector2D adj2(3, 2);
    QVector2D adj3(4, 1);
    adjustments.append(adj1);
    adjustments.append(adj2);
    adjustments.append(adj3);

    ScreenBBArrangement arrangement;
    QVector2D r =arrangement.combineAdjustments(adjustments);

    QCOMPARE(r.x(), 2.0f);
    QCOMPARE(r.y(), 1.0f);
}

void ScreenBBArrangementTest::combineAdjustments_emptyList_noAdjustment()
{
    QList<QVector2D> adjustments;
    ScreenBBArrangement arrangement;
    QVector2D r =arrangement.combineAdjustments(adjustments);

    QCOMPARE(r.x(), 0.0f);
    QCOMPARE(r.y(), 0.0f);
}
