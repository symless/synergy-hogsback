#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ScreenListModel.h"
#include <QMainWindow>
#include <QStringListModel>

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
    void on_p_listViewScreenNames_pressed(const QModelIndex &index);
    void on_p_pushButtonConnected_pressed();
    void on_p_pushButtonDisconnected_pressed();

private:
    void onReceivedDefaultMulticastMessage (MulticastMessage);
    void onReceivedUniqueMulticastMessage (MulticastMessage);
    Ui::MainWindow *ui;
    MulticastManager* m_multicastManager = nullptr;
    QStringListModel m_screenNamesModel;
    ScreenListModel m_screenListModel;
};

#endif // MAINWINDOW_H
