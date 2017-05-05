#ifndef PROFILE_H
#define PROFILE_H

#include <QObject>

class Profile: public QObject
{
    Q_OBJECT

public:
    Profile (int id = -1, QString name = ""): m_id(id), m_name(name) {}
    Profile (Profile const& src): m_id(src.m_id), m_name(src.m_name) {}

    Profile&
    operator= (Profile const& src) {
        m_id = src.m_id;
        m_name = src.m_name;
        return *this;
    }

    Q_PROPERTY (int pid READ getPid WRITE setPid NOTIFY idChanged)
    Q_PROPERTY (QString name READ getName WRITE setName NOTIFY nameChanged)

    Q_INVOKABLE int getPid() const {
        return m_id;
    }

    Q_INVOKABLE QString getName() const {
        return m_name;
    }


    Q_INVOKABLE void setPid(int id) {
        m_id = id;
    }

    Q_INVOKABLE void setName(QString name) {
        m_name = name;
    }

signals:
    void idChanged (int);
    void nameChanged (QString const&);

private:    
    int m_id;
    QString m_name;
};

Q_DECLARE_METATYPE(Profile)

#endif // PROFILE_H
