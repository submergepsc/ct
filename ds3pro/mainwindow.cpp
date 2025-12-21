#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include "coursedata.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::runSchedule);
    connect(ui->loadButton, &QPushButton::clicked, this, &MainWindow::loadFromFile);
    ui->strategyCombo->addItem(QStringLiteral("负载均衡"), static_cast<int>(SchedulingStrategy::BalanceLoad));
    ui->strategyCombo->addItem(QStringLiteral("前置集中"), static_cast<int>(SchedulingStrategy::Frontload));
    ui->strategyCombo->setCurrentIndex(0);

    ensureSampleCourseFile();
    ui->courseFileEdit->setText(defaultCoursesPath());
    loadCoursesFromFile(ui->courseFileEdit->text());
    ui->outputFileLabel->setText(defaultOutputPath());
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::defaultCoursesPath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("courses.txt"));
}

QString MainWindow::defaultOutputPath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("schedule_output.txt"));
}

void MainWindow::ensureSampleCourseFile()
{
    QString error;
    CourseIO::writeIfMissing(defaultCoursesPath(), CourseIO::defaultCourses(), error);
}

bool MainWindow::loadCoursesFromFile(const QString &path)
{
    QString content;
    QString error;
    if (!CourseIO::loadFile(path, content, error)) {
        QMessageBox::warning(this, QStringLiteral("读取失败"),
                             QStringLiteral("无法读取课程文件：%1\n%2").arg(path, error));
        return false;
    }
    ui->courseInput->setPlainText(content);
    return true;
}

QList<Course> MainWindow::parseCoursesFromText(const QString &text)
{
    return CourseIO::parse(text);
}

void MainWindow::runSchedule()
{
    bool okSemester = false;
    bool okCredit = false;
    const int semesters = ui->semesterEdit->text().toInt(&okSemester);
    const int creditLimit = ui->creditLimitEdit->text().toInt(&okCredit);
    if (!okSemester || !okCredit) {
        QMessageBox::warning(this, QStringLiteral("参数错误"),
                             QStringLiteral("请输入有效的整数参数。"));
        return;
    }

    const auto strategyValue = ui->strategyCombo->currentData().toInt();
    const auto strategy = static_cast<SchedulingStrategy>(strategyValue);

    Scheduler scheduler;
    try {
        const auto courses = parseCoursesFromText(ui->courseInput->toPlainText());
        const auto result = scheduler.schedule(semesters, creditLimit, courses, strategy);
        QStringList lines;
        for (const auto &semester : result.semesters) {
            QStringList courseTexts;
            for (const auto &course : semester.courses) {
                courseTexts << QStringLiteral("%1 %2(%3)").arg(course.id, course.name).arg(course.credits);
            }
            const QString courses = courseTexts.isEmpty() ? QStringLiteral("（本学期无可排课程）")
                                                          : courseTexts.join(QStringLiteral("、"));
            lines << QStringLiteral("学期 %1（总学分 %2）：%3")
                         .arg(semester.index)
                         .arg(semester.totalCredits())
                         .arg(courses);
        }
        lines << QStringLiteral("排课成功：%1 门课程全部在 %2 个学期内完成，总学分 %3。")
                     .arg(result.totalCourses())
                     .arg(result.semesters.size())
                     .arg(result.totalCredits());
        ui->outputBrowser->setText(lines.join('\n'));

        QFile outFile(defaultOutputPath());
        if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&outFile);
            out.setCodec("UTF-8");
            out << lines.join('\n');
            ui->outputStatusLabel->setText(QStringLiteral("已写入：%1").arg(outFile.fileName()));
        } else {
            ui->outputStatusLabel->setText(QStringLiteral("写入失败：%1").arg(outFile.errorString()));
        }
    } catch (const SchedulerError &err) {
        QMessageBox::critical(this, QStringLiteral("排课失败"), err.message());
        ui->outputBrowser->setText(QStringLiteral("排课失败：%1").arg(err.message()));
        ui->outputStatusLabel->setText(QStringLiteral("未生成输出文件"));
    }
}

void MainWindow::loadFromFile()
{
    const QString path = ui->courseFileEdit->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("路径为空"), QStringLiteral("请输入课程文件路径。"));
        return;
    }
    loadCoursesFromFile(path);
}
