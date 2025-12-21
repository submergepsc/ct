#include "coursedata.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

QList<Course> CourseIO::defaultCourses()
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

QList<Course> CourseIO::parse(const QString &text)
{
    QList<Course> courses;
    const QRegularExpression idRegex(QStringLiteral("^[A-Za-z0-9]{3}$"));
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

        if (!idRegex.match(c.id).hasMatch()) {
            throw SchedulerError(QStringLiteral("课程号格式无效（需 3 位字母或数字）：%1").arg(c.id));
        }
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

QString CourseIO::serialize(const QList<Course> &courses)
{
    QStringList lines;
    lines << QStringLiteral("# 课程号,课程名称,学分,先修课(以;分隔)");
    for (const auto &c : courses) {
        lines << QStringLiteral("%1,%2,%3,%4")
                     .arg(c.id, c.name)
                     .arg(c.credits)
                     .arg(c.prerequisites.join(';'));
    }
    return lines.join('\n');
}

bool CourseIO::loadFile(const QString &path, QString &content, QString &error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = file.errorString();
        return false;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    content = in.readAll();
    return true;
}

bool CourseIO::saveFile(const QString &path, const QString &content, QString &error)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        error = file.errorString();
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    return true;
}

bool CourseIO::writeIfMissing(const QString &path, const QList<Course> &courses, QString &error)
{
    QFile file(path);
    if (file.exists()) return true;
    const QString content = serialize(courses);
    return saveFile(path, content, error);
}
