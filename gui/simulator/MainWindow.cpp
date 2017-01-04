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
    connect(m_multicastManager, &MulticastManager::receivedUniqueGroupMessage,
             this, &MainWindow::onReceivedUniqueMulticastMessage);
    connect(m_multicastManager, &MulticastManager::receivedDefaultGroupMessage,
             this, &MainWindow::onReceivedDefaultMulticastMessage);

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

void
MainWindow::onReceivedDefaultMulticastMessage(MulticastMessage msg) {
    ui->p_textBrowserLogging->append("===== Received Default Message =====");
    ui->p_textBrowserLogging->append(msg.toReadableString());
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

void
MainWindow::onReceivedUniqueMulticastMessage(MulticastMessage msg) {
    ui->p_textBrowserLogging->append("===== Received Unique Message =====");
    ui->p_textBrowserLogging->append(msg.toReadableString());

    if (msg.m_type == MulticastMessage::kUniqueConfig ||
        msg.m_type == MulticastMessage::kUniqueConfigDelta) {
        ConfigMessageConvertor convertor;
        QList<Screen> screenList;
        // set serial to -1 to bypass serial checking
        int lastSerial = -1;
        if(convertor.fromStringToList(screenList,
                        msg.m_configInfo,
                        lastSerial,
                        msg.m_type == MulticastMessage::kUniqueConfig)) {

            // if there is only 1 screen in the list and its position is -1,-1,
            // it means remove this screen in configuration
            if (screenList.size() == 1) {
                if (screenList[0].posX() == -1 &&
                    screenList[0].posY() == -1) {
                    getScreenModel()->removeScreen(screenList[0].name());
                    return;
                }
            }

            getScreenModel()->update(screenList);
            syncNameAndScreenModel();
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
        if (!hostname.isEmpty()) {
            m_multicastManager->setLocalHostname(hostname);
            m_multicastManager->multicastUniqueLeave(kClientMode);
            m_multicastManager->leaveUniqueGroup();
        }
    }
}

void MainWindow::on_p_listViewScreenNames_pressed(const QModelIndex &index)
{
    QString selected = getNameModel()->data(index, Qt::DisplayRole).toString();
    ui->p_lineEditScreenName->setText(selected);
}

void MainWindow::on_p_pushButtonConnected_pressed()
{
    QString hostname = ui->p_lineEditScreenName->text();
    if (!hostname.isEmpty()) {
        int index = getScreenModel()->findScreen(hostname);
        const Screen& screen = getScreenModel()->getScreen(index);
        Screen screenCopy = screen;
        screenCopy.setState(kConnected);
        ConfigMessageConvertor convertor;
        QString data = convertor.fromScreenToString(screenCopy);
        m_multicastManager->multicastUniqueConfigDelta(data);
    }
}

void MainWindow::on_p_pushButtonDisconnected_pressed()
{
    QString hostname = ui->p_lineEditScreenName->text();
    if (!hostname.isEmpty()) {
        int index = getScreenModel()->findScreen(hostname);
        const Screen& screen = getScreenModel()->getScreen(index);
        Screen screenCopy = screen;
        screenCopy.setState(kDisconnected);
        ConfigMessageConvertor convertor;
        QString data = convertor.fromScreenToString(screenCopy);
        m_multicastManager->multicastUniqueConfigDelta(data);
    }
}

void MainWindow::on_p_comboBoxGroup_currentIndexChanged(int index)
{
    ui->p_listViewScreenNames->setModel(m_namesModelList[index]);
}
