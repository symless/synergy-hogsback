#pragma once

#include <map>

#include <stdint.h>

enum class ScreenError: uint32_t {
    kNone = 0,
    kRouterUnreachableNode = 0x10000001,
    kCoreZombieProcess = 0x20000001,
    kCoreDuplicateName = 0x20000002,
    kGeneral = 0x30000000
};

static std::map<ScreenError, std::string> screenErrorUrlMap = {
    {ScreenError::kRouterUnreachableNode, "connection"},
    {ScreenError::kCoreZombieProcess, "core"},
    {ScreenError::kCoreDuplicateName, "core"},
    {ScreenError::kGeneral, "general"}
};

static const char* kSynergyHelpUrl = "https://symless.com/synergy/help/";

static std::string
getDefaultErrorMessage()
{
    return "There seem to be an error in this screen.";
}

static std::string
getHelpUrl(ScreenError ec)
{
    auto it = screenErrorUrlMap.find(ec);
    std::string url;
    url += "<a href='";

    if (it != screenErrorUrlMap.end()) {
        url += kSynergyHelpUrl + it->second;
    }
    else {
        url += kSynergyHelpUrl;
        url += "general";
    }

    url += "'>Help</a>";
    return url;
}
