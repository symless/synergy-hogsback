#ifndef PROJECT_PROFILESNAPSHOT_H
#define PROJECT_PROFILESNAPSHOT_H

#include "synergy/common/ScreenStatus.h"
#include <vector>

class ProfileSnapshot
{
public:
    void parseJsonSnapshot(std::string const &jsonConfig);

    // TODO: ideally those structs should be private,
    // but private control accessiblity prevent them being used by boost fusion
public:
    struct Profile {
        int configVersion = -1;
        int id = -1;
        std::string name;
        int server = -1;
    };

    struct Screen {
        int id = -1;
        std::string name;
        std::string active;
        std::string ipList;
        std::string status;
        int x_pos = -1;
        int y_pos = -1;
    };

    struct Snapshot {
        Profile profile;
        std::vector<Screen> screens;
    };

private:
    Snapshot m_data;
};

#endif //PROJECT_PROFILESNAPSHOT_H
