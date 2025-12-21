#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

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
    QFile file(defaultCoursesPath());
    if (file.exists()) return;
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "# 课程号,课程名称,学分,先修课(以;分隔)\n";
        for (const auto &c : sampleCourses()) {
            out << c.id << "," << c.name << "," << c.credits << ",";
            out << c.prerequisites.join(";") << "\n";
        }
    }
}

bool MainWindow::loadCoursesFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("读取失败"),
                             QStringLiteral("无法读取课程文件：%1").arg(path));
        return false;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    ui->courseInput->setPlainText(in.readAll());
    return true;
}

QList<Course> MainWindow::sampleCourses() const
{
    return {
        {"c1", "程序设计基础", 2, {}},
        {"c2", "高等数学", 3, {"c1"}},
        {"c3", "数据结构", 4, {"c1"}},
        {"c4", "汇编语言", 3, {"c1"}},
        {"c5", "高级语言程序设计", 2, {"c3", "c4"}},
        {"c6", "计算机原理", 4, {"c2", "c4"}},
        {"c7", "编译原理", 3, {"c3", "c5"}},
        {"c8", "操作系统", 4, {"c5", "c6"}},
        {"c9", "软件工程", 7, {"c7", "c8"}},
        {"c10", "计算机网络", 5, {"c6"}},
        {"c11", "数值计算", 2, {"c2"}},
        {"c12", "数值分析", 3, {"c1", "c6", "c10"}},
    };
}

QList<Course> MainWindow::parseCoursesFromText(const QString &text)
{
    QList<Course> courses;
    const auto lines = text.split('\n', Qt::SkipEmptyParts);
    for (const auto &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("#") || trimmed.isEmpty()) continue;

        const auto parts = trimmed.split(',');
        if (parts.size() < 3) {
            throw SchedulerError(QStringLiteral("课程行缺少字段：%1").arg(trimmed));
        }
        Course c;
        c.id = parts.at(0).trimmed();
        c.name = parts.at(1).trimmed();
        bool okCredits = false;
        c.credits = parts.at(2).trimmed().toInt(&okCredits);
        if (!okCredits) {
            throw SchedulerError(QStringLiteral("课程 %1 学分无效").arg(c.id));
        }
        if (parts.size() >= 4) {
            const auto prereqStr = parts.at(3).trimmed();
            if (!prereqStr.isEmpty()) {
                c.prerequisites = prereqStr.split(';', Qt::SkipEmptyParts);
                for (auto &p : c.prerequisites) p = p.trimmed();
            }
        }
        courses.push_back(c);
    }
    if (courses.isEmpty()) {
        throw SchedulerError(QStringLiteral("课程列表为空"));
    }
    return courses;
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
