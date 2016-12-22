#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
	void onUniqueGroupMessage (MulticastMessage);
	Ui::MainWindow *ui;
	MulticastManager* m_multicastManager = nullptr;
};

#endif // MAINWINDOW_H
