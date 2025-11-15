#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ItemModel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void addItem();
    void removeSelected();
    void clearItems();
    void updateSummary();

private:
    void connectSignals();
    void resetForm();

    Ui::MainWindow *ui;
    ItemModel *m_model;
};

#endif // MAINWINDOW_H
