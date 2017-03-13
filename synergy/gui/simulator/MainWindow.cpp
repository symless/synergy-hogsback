#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ProcessMode.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for (int i = 0; i < ui->p_comboBoxGroup->count(); i++) {
        QStringListModel* name = new QStringListModel();
        m_namesModelList.append(name);

        ScreenListModel* model = new ScreenListModel();
        m_screemModelList.append(model);
    }

    ui->p_listViewScreenNames->setModel(m_namesModelList[0]);
}

MainWindow::~MainWindow()
{
    delete ui;

    for (int i = 0; i < ui->p_comboBoxGroup->count(); i++) {
        delete m_namesModelList[i];
        delete m_screemModelList[i];
    }
}

void MainWindow::syncNameAndScreenModel()
{
    int index = ui->p_comboBoxGroup->currentIndex();
    QStringListModel* nameModel = m_namesModelList[index];
    ScreenListModel* screenModel = m_screemModelList[index];

    foreach (const Screen& screen, screenModel->getScreenSet()) {
        if (!nameModel->stringList().contains(screen.name())) {
            nameModel->insertRow(nameModel->rowCount());
            QModelIndex i = nameModel->index(nameModel->rowCount() - 1);
            nameModel->setData(i, screen.name());
        }
    }

}

QStringListModel* MainWindow::getNameModel()
{
    return m_namesModelList[ui->p_comboBoxGroup->currentIndex()];
}

ScreenListModel* MainWindow::getScreenModel()
{
    return m_screemModelList[ui->p_comboBoxGroup->currentIndex()];
}

void MainWindow::on_p_pushButtonAdd_clicked()
{
    // insert the screen name into the list view, no duplicate checking atm
    getNameModel()->insertRow(getNameModel()->rowCount());
    QModelIndex index = getNameModel()->index(getNameModel()->rowCount() - 1);
    getNameModel()->setData(index, ui->p_lineEditScreenName->text());
}

void MainWindow::on_p_pushButtonRemove_clicked()
{
    // get the index number of the screen name shown in screen name field
    int index = getNameModel()->stringList().indexOf(
                    ui->p_lineEditScreenName->text());
    getNameModel()->removeRow(index);
}

void MainWindow::on_p_pushButtonJoin_clicked()
{
    int index = ui->p_comboBoxGroup->currentIndex();

    // want to join default group, as we automatically join default group,
    // this means we want to know the information of server existance
}

void MainWindow::on_p_pushButtonLeave_clicked()
{
    int index = ui->p_comboBoxGroup->currentIndex();
}

void MainWindow::on_p_listViewScreenNames_pressed(const QModelIndex &index)
{
    QString selected = getNameModel()->data(index, Qt::DisplayRole).toString();
    ui->p_lineEditScreenName->setText(selected);
}

void MainWindow::on_p_pushButtonConnected_pressed()
{
    QString hostname = ui->p_lineEditScreenName->text();
}

void MainWindow::on_p_pushButtonDisconnected_pressed()
{
    QString hostname = ui->p_lineEditScreenName->text();
}

void MainWindow::on_p_comboBoxGroup_currentIndexChanged(int index)
{
    ui->p_listViewScreenNames->setModel(m_namesModelList[index]);
}
