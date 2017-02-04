#ifndef CONFIGMESSAGECONVERTORTEST_H
#define CONFIGMESSAGECONVERTORTEST_H

#include "AutoTest.h"

#include <QObject>

class ConfigMessageConvertorTest : public QObject
{
    Q_OBJECT
public:
    explicit ConfigMessageConvertorTest(QObject *parent = 0);

private slots:
    void fromModelToString_emptyModel_returnZero();
    void fromModelToString_validModel_returnAllScreensInfo();
    void fromScreenToString_validScreen_returnScreenInfoString();
    void fromStringToList_validFullConfigLatestSerial_updateScreenList();
    void fromStringToList_validFullConfigOutdatedSerial_returnFalse();
    void fromStringToList_invalidFullConfig_returnFalse();
    void fromStringToList_validIncompleteConfigLatestSerial_updateScreenList();
    void fromStringToList_validDeltaConfig_updateScreenList();
    void fromStringToList_invalidDeltaConfig_returnFalse();
};

DECLARE_TEST(ConfigMessageConvertorTest)

#endif // CONFIGMESSAGECONVERTORTEST_H
