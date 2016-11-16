#ifndef MOCKSCREENMODEL_H
#define MOCKSCREENMODEL_H

#include "ScreenModel.h"

class MockScreenModel : public ScreenModel
{
public:
	MockScreenModel();

	virtual void adjustAll(int disX, int disY);

public:
	int m_disX;
	int m_disY;
};

#endif // MOCKSCREENMODEL_H
