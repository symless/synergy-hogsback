#include "TrialValidator.h"

#include "Macro.h"
#include <QDate>
#include <QString>

const int kTrialDays = 30;

TrialValidator::TrialValidator()
{

}

bool TrialValidator::isValid()
{
    QString dateString(STRINGIZE(SYNERGY_BUILD_DATE));
    QDate buildDate = QDate::fromString(dateString, "yyyyMMdd");
    QDate currentDate = QDate::currentDate();
    QDate expiredDate = buildDate.addDays(kTrialDays);
    return currentDate < expiredDate;
}
