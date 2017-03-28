#ifndef SCREENlISTMODEL_H
#define SCREENlISTMODEL_H

#include "LibMacro.h"
#include "Screen.h"
#include <QAbstractListModel>

class LIB_SPEC ScreenListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ScreenRoles {
        kPosXRole = Qt::UserRole + 1,
        kPosYRole,
        kNameRole,
        kStateImageRole
    };

    ScreenListModel();
    virtual ~ScreenListModel();

    Q_PROPERTY(float scale READ scale NOTIFY scaleChanged)

    Q_INVOKABLE int screenIconWidth();
    Q_INVOKABLE int screenIconHeight();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QModelIndex getIndex(int row, int column = 0,
            const QModelIndex& parent = QModelIndex()) const;

    int findScreen(QString name);
    void addScreen(const Screen& screen);
    void removeScreen(QString name);
    void getScreenPos(QString name, int& newPosX,int& newPosY);
    virtual void adjustAll(int disX, int disY);
    int getModelIndex(int x, int y);
    void moveModel(int index, int offsetX, int offsetY);
    int getScreenModeSize();
    const Screen& getScreen(int index) const;
    void update(const QList<Screen>& screens);
    QSet<Screen> getScreenSet();
    float scale() const;
    void setScale(float s);
    QList<Screen> getScreenList() const;
    void lockScreen(int index);
    void unlockScreen(int index);

signals:
    void scaleChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<Screen> m_screens;
    float m_scale;
};

#endif // SCREENlISTMODEL_H
