#include "ScreenListModel.h"

#include "Screen.h"
#include "Common.h"

#include <QSet>

ScreenListModel::ScreenListModel() :
	m_scale(1.0f)
{
}

ScreenListModel::~ScreenListModel()
{

}

int ScreenListModel::screenIconWidth()
{
	return kScreenIconWidth;
}

int ScreenListModel::screenIconHeight()
{
	return kScreenIconHeight;
}

int ScreenListModel::getModelIndex(int x, int y)
{
	for (int index = 0; index < m_screens.count(); index++) {
		int posX = m_screens[index].posX();
		int posY = m_screens[index].posY();

		if (posX <= x && posX + kScreenIconWidth >= x &&
			posY <= y && posY + kScreenIconHeight >= y) {
			return index;
		}
	}

	return -1;
}

void ScreenListModel::moveModel(int index, int offsetX, int offsetY)
{
	if (index < 0 && index >= m_screens.count()) {
		return;
	}

	int x = m_screens[index].posX();
	int y = m_screens[index].posY();
	m_screens[index].setPosX(x + offsetX);
	m_screens[index].setPosY(y + offsetY);

	dataChanged(getIndex(index), getIndex(index));
}

int ScreenListModel::getScreenModeSize()
{
	return m_screens.count();
}

const Screen& ScreenListModel::getScreen(int index) const
{
	return m_screens[index];
}

void ScreenListModel::update(const QList<Screen>& screens)
{
	for (int i = 0; i < screens.count(); i++) {
		int r = findScreen(screens[i].name());
		if (r != -1) {
			m_screens[r] = screens[i];
			dataChanged(getIndex(r), getIndex(r));
		}
		else {
			addScreen(screens[i]);
		}
    }
}

QSet<Screen> ScreenListModel::getScreenNames()
{
    return QSet<Screen>::fromList(m_screens);
}

int ScreenListModel::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return m_screens.count();
}

QVariant ScreenListModel::data(const QModelIndex& index, int role) const
{
	if (index.row() < 0 || index.row() >= m_screens.count())
		return QVariant();

	const Screen& screen = m_screens[index.row()];
	if (role == kPosXRole)
		return screen.posX();
	else if (role == kPosYRole)
		return screen.posY();
	else if (role == kNameRole)
		return screen.name();
	else if (role == kStateImageRole)
		return screen.stateImage();

	return QVariant();
}

QModelIndex ScreenListModel::getIndex(int row, int column,
								 const QModelIndex& parent) const
{
	return hasIndex(row, column, parent) ?
				createIndex(row, column, (void*)&m_screens[row])
				: QModelIndex();
}

QHash<int, QByteArray> ScreenListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[kPosXRole] = "posX";
	roles[kPosYRole] = "posY";
	roles[kNameRole] = "name";
	roles[kStateImageRole] = "stateImage";

	return roles;
}

int ScreenListModel::findScreen(QString name)
{
	for (int index = 0; index < m_screens.count(); index++) {
		if (m_screens[index].name() == name) return index;
	}

	return -1;
}

void ScreenListModel::addScreen(const Screen& screen)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_screens.append(screen);
	endInsertRows();
}

void ScreenListModel::removeScreen(QString name)
{
	int index = findScreen(name);
	beginRemoveRows(QModelIndex(), index, index);
	m_screens.removeAt(index);
	endRemoveRows();
}

void ScreenListModel::getScreenPos(QString name, int& newPosX, int& newPosY)
{
	for (int index = 0; index < m_screens.count(); index++) {
		if (m_screens[index].name() == name) {
			newPosX = m_screens[index].posX();
			newPosY = m_screens[index].posY();
			return;
		}
	}
}

void ScreenListModel::adjustAll(int disX, int disY)
{
	for (int index = 0; index < m_screens.count(); index++) {
		m_screens[index].setPosX(m_screens[index].posX() + disX);
		m_screens[index].setPosY(m_screens[index].posY() + disY);
	}

	dataChanged(getIndex(0), getIndex(rowCount() - 1));
}

float ScreenListModel::scale() const
{
	return m_scale;
}

void ScreenListModel::setScale(float s)
{
	m_scale = s;
	emit scaleChanged();
}

