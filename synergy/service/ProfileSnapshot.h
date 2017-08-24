#ifndef PROJECT_PROFILESNAPSHOT_H
#define PROJECT_PROFILESNAPSHOT_H

#include "synergy/common/ScreenStatus.h"

class ProfileSnapshot
{
public:
    void parseJsonSnapshot(std::string const &jsonConfig);
};

#endif //PROJECT_PROFILESNAPSHOT_H
