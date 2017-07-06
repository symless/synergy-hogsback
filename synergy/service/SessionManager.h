#ifndef COMMONSESSIONMANAGER_H
#define COMMONSESSIONMANAGER_H

#include <string>
#include <boost/signals2.hpp>

class SessionManager
{
public:
    boost::signals2::signal<void (int)> sessionChanged;

private:
    int m_currentSessionId;
    int getSessionId() { return m_currentSessionId + 1; }

    void loop() {
        while (true) {
//            sleep(1);
//            auto sessionId = getSessionId();
//            if (m_currentSessionId != sessionId) {
//                m_currentSessionId = sessionId;
//                sessionChanged (m_currentSessionId);
//            }
        }
    }
};

#endif // COMMONSESSIONMANAGER_H
