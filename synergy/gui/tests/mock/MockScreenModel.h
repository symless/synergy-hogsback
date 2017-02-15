#ifndef MOCKSCREENMODEL_H
#define MOCKSCREENMODEL_H

#include "ScreenListModel.h"

class MockScreenModel : public ScreenListModel
{
public:
    MockScreenModel();

    virtual void adjustAll(int disX, int disY);

public:
    int m_disX;
    int m_disY;
};

#endif // MOCKSCREENMODEL_H
