#pragma once

#include <synergy/common/Screen.h>
#include <synergy/common/ScreenLinks.h>
#include <vector>
#include <ostream>

const std::string kCoreConfigFile = "synergy.conf";

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

void
createConfigFile(const std::string& path,
                 const std::vector<Screen>& screens);
