#include <synergy/common/Screen.h>
#include <synergy/common/ScreenLinks.h>
#include <vector>
#include <ostream>

std::vector<ScreenLinks>
linkScreens (std::vector<Screen>& screens);

void
printScreenLinks (std::ostream& os, std::vector<Screen> const& screens,
                  std::vector<ScreenLinks> const& links, int i);

void
printScreenLinks (std::ostream& os, ScreenLinkMap const& screen,
                  std::vector<ScreenLinkMap> const& targets);
