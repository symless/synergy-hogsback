#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MulticastManager.h"
#include "ProcessMode.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	m_multicastManager = qobject_cast<MulticastManager*>
								(MulticastManager::instance());
	ui->setupUi(this);
	connect (m_multicastManager, &MulticastManager::receivedUniqueGroupMessage,
			 this, &MainWindow::onReceivedMulticastMessage);
	connect (m_multicastManager, &MulticastManager::receivedDefaultGroupMessage,
			 this, &MainWindow::onReceivedMulticastMessage);

	ui->p_listViewScreenNames->setModel(&m_screenNamesModel);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void
MainWindow::onReceivedMulticastMessage(MulticastMessage msg) {
	//ui->p_comboBoxGroup->addItem ("test: " + msg.m_hostname);
}

void MainWindow::on_p_pushButtonAdd_clicked()
{
	// insert the screen name into the list view, no duplicate checking atm
	m_screenNamesModel.insertRow(m_screenNamesModel.rowCount());
	QModelIndex index = m_screenNamesModel.index(m_screenNamesModel.rowCount() - 1);
	m_screenNamesModel.setData(index, ui->p_lineEditScreenName->text());
}

void MainWindow::on_p_pushButtonRemove_clicked()
{
	// get the index number of the screen name shown in screen name field
	int index = m_screenNamesModel.stringList().indexOf(
					ui->p_lineEditScreenName->text());
	m_screenNamesModel.removeRow(index);
}

void MainWindow::on_p_pushButtonJoin_clicked()
{
	int index = ui->p_comboBoxGroup->currentIndex();

	// want to join default group, as we automatically join default group,
	// this means we want to know the information of server existance
	if (index == 0) {
		m_multicastManager->multicastDefaultExistence();
	}
	// join unique group
	else {
		QString hostname = ui->p_lineEditScreenName->text();
		if (!ui->p_lineEditScreenName->text().isEmpty()) {
			m_multicastManager->setLocalHostname(hostname);
			m_multicastManager->joinUniqueGroup(index);
			m_multicastManager->multicastUniqueJoin(kClientMode);
		}
	}
}

void MainWindow::on_p_pushButtonLeave_clicked()
{
	int index = ui->p_comboBoxGroup->currentIndex();

	// only leave unique groups
	if (index != 0) {
		QString hostname = ui->p_lineEditScreenName->text();
		if (!ui->p_lineEditScreenName->text().isEmpty()) {
			m_multicastManager->setLocalHostname(hostname);
			m_multicastManager->multicastUniqueLeave(kClientMode);
			m_multicastManager->leaveUniqueGroup();
		}
	}
}

void MainWindow::on_p_listViewScreenNames_pressed(const QModelIndex &index)
{
	QString selected = m_screenNamesModel.data(index, Qt::DisplayRole).toString();
	ui->p_lineEditScreenName->setText(selected);
}
