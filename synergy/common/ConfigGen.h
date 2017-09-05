#include <synergy/common/Screen.h>
#include <synergy/common/ScreenLinks.h>
#include <vector>
#include <ostream>

std::vector<ScreenLinks>
linkScreens (std::vector<Screen> const& screens);

void
printScreenLinks (std::ostream& os, std::vector<Screen> const& screens,
                  std::vector<ScreenLinks>& links, int i);

void
printScreenLinks (std::ostream& os, ScreenLinkMap const& screen,
                  std::vector<ScreenLinkMap> const& targets);

void
printConfig (std::ostream& os, std::vector<Screen> const& screens);
