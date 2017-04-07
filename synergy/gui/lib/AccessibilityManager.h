#ifndef ACCESSIBILITYMANAGER_H
#define ACCESSIBILITYMANAGER_H

#include <QObject>

class AccessibilityManager : public QObject
{
    Q_OBJECT
public:
    explicit AccessibilityManager (QObject *parent = 0);

    Q_INVOKABLE bool processHasAccessibility() const;
    Q_INVOKABLE void openAccessibilityDialog() const;
signals:

public slots:
};

#endif // ACCESSIBILITYMANAGER_H
