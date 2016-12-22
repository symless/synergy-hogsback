#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QSet>

namespace Ui {
class MainWindow;
}

class MulticastMessage;
class MulticastManager;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_p_pushButtonAdd_clicked();
	void on_p_pushButtonRemove_clicked();
	void on_p_pushButtonJoin_clicked();
	void on_p_pushButtonLeave_clicked();

private:
	void onReceivedMulticastMessage (MulticastMessage);
	Ui::MainWindow *ui;
	MulticastManager* m_multicastManager = nullptr;
	QStringListModel m_screenNamesModel;
	QSet<QString> m_screenNames;
};

#endif // MAINWINDOW_H
