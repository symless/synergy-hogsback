#include "AccessibilityManager.h"

#ifdef __APPLE__
extern bool processHasAccessibility();
extern void openAccessibilityDialog();
#endif

AccessibilityManager::AccessibilityManager(QObject *parent) : QObject(parent)
{

}

bool
AccessibilityManager::processHasAccessibility() const
{
#ifdef __APPLE__
    return ::processHasAccessibility();
#else
    return true;
#endif
}

void
AccessibilityManager::openAccessibilityDialog() const
{
#ifdef __APPLE__
    ::openAccessibilityDialog();
#endif
}
