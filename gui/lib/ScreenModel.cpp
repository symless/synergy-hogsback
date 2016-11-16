#include "ScreenModel.h"

#include "Screen.h"
#include "Common.h"

ScreenModel::ScreenModel() :
	m_scale(1.0f)
{
}

ScreenModel::~ScreenModel()
{

}

int ScreenModel::screenIconWidth()
{
	return kScreenIconWidth;
}

int ScreenModel::screenIconHeight()
{
	return kScreenIconHeight;
}

int ScreenModel::getModelIndex(int x, int y)
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

void ScreenModel::moveModel(int index, int offsetX, int offsetY)
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

int ScreenModel::getScreenModeSize()
{
	return m_screens.count();
}

const Screen& ScreenModel::getScreen(int index) const
{
	return m_screens[index];
}

void ScreenModel::update(const QList<Screen>& screens)
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

int ScreenModel::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return m_screens.count();
}

QVariant ScreenModel::data(const QModelIndex& index, int role) const
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

QModelIndex ScreenModel::getIndex(int row, int column,
								 const QModelIndex& parent) const
{
	return hasIndex(row, column, parent) ?
				createIndex(row, column, (void*)&m_screens[row])
				: QModelIndex();
}

QHash<int, QByteArray> ScreenModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[kPosXRole] = "posX";
	roles[kPosYRole] = "posY";
	roles[kNameRole] = "name";
	roles[kStateImageRole] = "stateImage";

	return roles;
}

int ScreenModel::findScreen(QString name)
{
	for (int index = 0; index < m_screens.count(); index++) {
		if (m_screens[index].name() == name) return index;
	}

	return -1;
}

void ScreenModel::addScreen(const Screen& screen)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_screens.append(screen);
	endInsertRows();
}

void ScreenModel::removeScreen(QString name)
{
	int index = findScreen(name);
	beginRemoveRows(QModelIndex(), index, index);
	m_screens.removeAt(index);
	endRemoveRows();
}

void ScreenModel::getScreenPos(QString name, int& newPosX, int& newPosY)
{
	for (int index = 0; index < m_screens.count(); index++) {
		if (m_screens[index].name() == name) {
			newPosX = m_screens[index].posX();
			newPosY = m_screens[index].posY();
			return;
		}
	}
}

void ScreenModel::adjustAll(int disX, int disY)
{
	for (int index = 0; index < m_screens.count(); index++) {
		m_screens[index].setPosX(m_screens[index].posX() + disX);
		m_screens[index].setPosY(m_screens[index].posY() + disY);
	}

	dataChanged(getIndex(0), getIndex(rowCount() - 1));
}

float ScreenModel::scale() const
{
	return m_scale;
}

void ScreenModel::setScale(float s)
{
	m_scale = s;
	emit scaleChanged();
}

