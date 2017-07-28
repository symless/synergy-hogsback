#include "TrialValidator.h"

#include <QDate>
#include <QString>

const int kTrialDays = 90;

TrialValidator::TrialValidator()
{

}

bool TrialValidator::isValid()
{
#ifndef SYNERGY_BUILD_DATE
    return true;
#endif
    QString dateString(SYNERGY_BUILD_DATE);
    QDate buildDate = QDate::fromString(dateString, "yyyyMMdd");
    QDate currentDate = QDate::currentDate();
    QDate expiredDate = buildDate.addDays(kTrialDays);
    return currentDate < expiredDate;
}
