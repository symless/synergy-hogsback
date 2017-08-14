#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include <map>
#include <string>

enum ErrorCode {
    kNoError,
    kConnectionTimeout
};

static struct ErrorMessage {
    ErrorCode errorcode;
    std::string detail;
    std::string troubleshootUrl;
}
errorMessageTable[] {
    {kConnectionTimeout, "Can't send your passwords to Nick", "timeout-connecting"}
};

static const char* kSynergyHelpUrl = "https://symless.com/synergy/help/";

static std::string
getErrorMessage(ErrorCode ec)
{
    for (auto entry : errorMessageTable) {
        if (entry.errorcode == ec) {
            return entry.detail;
        }
    }
    return "";
}

static std::string
getHelpUrl(ErrorCode ec)
{
    for (auto entry : errorMessageTable) {
        if (entry.errorcode == ec) {
            std::string url;
            url += "<a href='";
            url += kSynergyHelpUrl + entry.troubleshootUrl;
            url += "'>Help</a>";

            return url;
        }
    }
    return "";
}



#endif // ERRORMESSAGE_H
