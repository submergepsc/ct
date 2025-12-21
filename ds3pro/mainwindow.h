#pragma once

#include <QWidget>

#include "scheduler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void runSchedule();

private:
    QList<Course> sampleCourses() const;

    Ui::MainWindow *ui;
};
