#include <QApplication>
#include <QMessageBox>
#include <QWidget>

#include "scheduler.h"
#include "ui_schedulerwidget.h"

static QList<Course> sampleCourses()
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

class SchedulerWidget : public QWidget {
    Q_OBJECT
public:
    SchedulerWidget(QWidget *parent = nullptr) : QWidget(parent) {
        ui.setupUi(this);
        connect(ui.runButton, &QPushButton::clicked, this, &SchedulerWidget::runSchedule);
    }

private slots:
    void runSchedule() {
        bool okSemester = false;
        bool okCredit = false;
        const int semesters = ui.semesterEdit->text().toInt(&okSemester);
        const int creditLimit = ui.creditLimitEdit->text().toInt(&okCredit);
        if (!okSemester || !okCredit) {
            QMessageBox::warning(this, QStringLiteral("参数错误"),
                                 QStringLiteral("请输入有效的整数参数。"));
            return;
        }

        Scheduler scheduler;
        try {
            const auto result = scheduler.schedule(semesters, creditLimit, sampleCourses());
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
            ui.outputBrowser->setText(lines.join('\n'));
        } catch (const SchedulerError &err) {
            QMessageBox::critical(this, QStringLiteral("排课失败"), err.message());
            ui.outputBrowser->setText(QStringLiteral("排课失败：%1").arg(err.message()));
        }
    }

private:
    Ui::SchedulerWidget ui;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SchedulerWidget widget;
    widget.setWindowTitle(QStringLiteral("教学计划编排"));
    widget.resize(520, 400);
    widget.show();

    return app.exec();
}

#include "main.moc"
