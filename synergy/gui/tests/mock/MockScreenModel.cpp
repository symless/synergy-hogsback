#include "MockScreenModel.h"

MockScreenModel::MockScreenModel() :
    m_disX(0),
    m_disY(0)
{

}

void MockScreenModel::adjustAll(int disX, int disY)
{
    m_disX = disX;
    m_disY = disY;
}
