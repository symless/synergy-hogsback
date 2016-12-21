#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MulticastManager.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	m_multicastManager = qobject_cast<MulticastManager*>
								(MulticastManager::instance());
	ui->setupUi(this);
	connect (m_multicastManager, &MulticastManager::receivedUniqueGroupMessage,
			 this, &MainWindow::onUniqueGroupMessage);
	connect (m_multicastManager, &MulticastManager::receivedDefaultGroupMessage,
			 this, &MainWindow::onUniqueGroupMessage);
	m_multicastManager->joinUniqueGroup(1);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void
MainWindow::onUniqueGroupMessage(MulticastMessage msg) {
	ui->comboBox->addItem ("test: " + msg.m_hostname);
}
