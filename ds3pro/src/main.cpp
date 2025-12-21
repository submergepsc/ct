#include <QCoreApplication>
#include <QTextStream>

#include "scheduler.h"

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
        {"c12", "数值分析", 3, {"c1", "c6", "c10"}}
    };
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);
    Scheduler scheduler;

    const int semesterCount = 6;
    const int creditLimit = 10;

    try {
        auto result = scheduler.schedule(semesterCount, creditLimit, sampleCourses());
        for (const auto &semester : result.semesters) {
            QStringList courseTexts;
            for (const auto &course : semester.courses) {
                courseTexts << QStringLiteral("%1 %2(%3)").arg(course.id, course.name).arg(course.credits);
            }
            const QString courses = courseTexts.isEmpty() ? QStringLiteral("（本学期无可排课程）")
                                                          : courseTexts.join(QStringLiteral("、"));
            out << QStringLiteral("学期 %1（总学分 %2）：%3\n")
                       .arg(semester.index)
                       .arg(semester.totalCredits())
                       .arg(courses);
        }
        out << QStringLiteral("排课成功：%1 门课程全部在 %2 个学期内完成，总学分 %3。\n")
                   .arg(result.totalCourses())
                   .arg(result.semesters.size())
                   .arg(result.totalCredits());
    } catch (const SchedulerError &err) {
        QTextStream errOut(stderr);
        errOut << QStringLiteral("排课失败：%1\n").arg(err.message());
        return 1;
    }

    return 0;
}
