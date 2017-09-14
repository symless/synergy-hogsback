#ifndef PROFILE_H
#define PROFILE_H

#include <QString>

class Profile
{
public:
    Profile (int id = -1, QString name = ""): m_id(id), m_name(name) {}
    int id() const { return m_id; }
    QString name() const { return m_name; }
private:    
    int m_id;
    QString m_name;
};

#endif // PROFILE_H
