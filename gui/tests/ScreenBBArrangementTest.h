#include "AutoTest.h"

class ScreenBBArrangementTest : public QObject
{
	Q_OBJECT
public:
	explicit ScreenBBArrangementTest(QObject* parent = 0);

private slots:
	void calculateNewPos_firstNewScreen_addToTheCenter();
	void calculateNewPos_firstExistScreen_noChangeInScreen();
	void calculateNewPos_newScreen_alignToLeft();
	void calculateNewPos_newScreen_alignToRight();
	void calculateNewPos_newScreen_alignToUp();
	void calculateNewPos_newScreen_alignToDown();

	void checkAdjustment_outsideOfView_noScale();
	void checkAdjustment_outsideOfView_scale();
	void checkAdjustment_viewTooBig_scale();
	void checkAdjustment_overScale_noScale();

	void getScreensNextTo_nonexistOnLeft_emptyList();
	void getScreensNextTo_oneExistOnLeft_listWithOneIndex();
	void getScreensNextTo_nonexistOnRight_emptyList();
	void getScreensNextTo_twoExistOnRight_listWithTwoIndices();
	void getScreensNextTo_nonexistOnTop_emptyList();
	void getScreensNextTo_oneExistOnTop_listWithOneIndex();
	void getScreensNextTo_nonexistOnBottom_emptyList();
	void getScreensNextTo_twoExistOnBottom_listWithTwoIndices();

	void recalculateBoundingBox_oneScreen_newBoundingBox();
	void recalculateBoundingBox_twoScreen_newBoundingBox();
	void recalculateBoundingBox_emptyModel_zeroBoundingBox();
	void recalculateBoundingBox_skipOne_zeroBoundingBox();
	void recalculateBoundingBox_skipOneInTwo_zeroBoundingBox();

	void adjustToCenter_oneScreen_moveScreenToCenter();

	void collide_overlap_returnTrue();
	void collide_noOverlap_returnFalse();

	void collideWithOthers_overlap_returOverlapList();
	void collideWithOthers_noOverlap_emptyList();

	void testSnapToOnAxis_withinLeftInnerThreshold_returnTrue();
	void testSnapToOnAxis_withinLeftOutsideThreshold_returnTrue();
	void testSnapToOnAxis_withinRightInnerThreshold_returnTrue();
	void testSnapToOnAxis_withinRightOutsideThreshold_returnTrue();
	void testSnapToOnAxis_outsideLeftOutsideThreshold_returnfalse();
	void testSnapToOnAxis_withinTwoInnerThresholds_returnfalse();
	void testSnapToOnAxis_outsideRightOutsideThreshold_returnfalse();

	void calculateSnapping_onlyCloseToRight_snapToRight();
	void calculateSnapping_closeToRightMoreThanTop_snapToRight();
	void calculateSnapping_onlyCloseToLeft_snapToLeft();
	void calculateSnapping_closeToLeftMoreThanBottom_snapToLeft();
	void calculateSnapping_onlyCloseToTop_snapToTop();
	void calculateSnapping_closeToTopMoreThanLeft_snapToTop();
	void calculateSnapping_onlyCloseToBottom_snapToBottom();
	void calculateSnapping_closeToBottomMoreThanRight_snapToBottom();

	void combineAdjustments_noMoveOnY_returnMinimumMoveOnX();
	void combineAdjustments_noMoveOnX_returnMinimumMoveOnY();
	void combineAdjustments_onlyYMovementExceedSnapThreshold_returnMinimumMoveOnX();
	void combineAdjustments_onlyXMovementExceedSnapThreshold_returnMinimumMoveOnY();
	void combineAdjustments_allWithinSnapThreshold_returnBothAdjustments();
	void combineAdjustments_emptyList_noAdjustment();
};

DECLARE_TEST(ScreenBBArrangementTest)
