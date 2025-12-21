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
    void loadFromFile();

private:
    QString defaultCoursesPath() const;
    QString defaultOutputPath() const;
    void ensureSampleCourseFile();
    bool loadCoursesFromFile(const QString &path);
    QList<Course> parseCoursesFromText(const QString &text);

    Ui::MainWindow *ui;
};
