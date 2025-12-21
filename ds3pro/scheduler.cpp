#include "scheduler.h"

#include <algorithm>
#include <functional>

int SemesterPlan::totalCredits() const {
    int sum = 0;
    for (const auto &course : courses) {
        sum += course.credits;
    }
    return sum;
}

int ScheduleResult::totalCredits() const {
    int sum = 0;
    for (const auto &semester : semesters) {
        sum += semester.totalCredits();
    }
    return sum;
}

int ScheduleResult::totalCourses() const {
    int sum = 0;
    for (const auto &semester : semesters) {
        sum += semester.courses.size();
    }
    return sum;
}

ScheduleResult Scheduler::schedule(int semesterCount,
                                   int creditLimit,
                                   const QList<Course> &courses,
                                   SchedulingStrategy strategy) const {
    if (semesterCount <= 0) {
        throw SchedulerError("学期数量必须大于 0");
    }
    if (semesterCount > 12) {
        throw SchedulerError("学期数量不能超过 12");
    }
    if (creditLimit <= 0) {
        throw SchedulerError("每学期学分上限必须大于 0");
    }
    if (courses.size() > 100) {
        throw SchedulerError("课程总数不能超过 100");
    }

    // basic validation and duplicate checks
    QSet<QString> ids;
    QSet<QString> names;
    int totalCredits = 0;
    for (const auto &course : courses) {
        if (course.credits <= 0) {
            throw SchedulerError(QStringLiteral("课程 %1 的学分必须大于 0").arg(course.id));
        }
        if (ids.contains(course.id)) {
            throw SchedulerError(QStringLiteral("检测到重复的课程编号：%1").arg(course.id));
        }
        ids.insert(course.id);
        if (names.contains(course.name)) {
            throw SchedulerError(QStringLiteral("检测到重复的课程名称：%1").arg(course.name));
        }
        names.insert(course.name);
        totalCredits += course.credits;
    }
    if (totalCredits > 100) {
        throw SchedulerError("课程总学分不能超过 100");
    }

    // prerequisite existence check
    for (const auto &course : courses) {
        for (const auto &pre : course.prerequisites) {
            if (!ids.contains(pre)) {
                throw SchedulerError(QStringLiteral("课程 %1 的先修课程不存在：%2").arg(course.id, pre));
            }
        }
    }

    if (hasCycle(courses)) {
        throw SchedulerError("先修关系存在环路，无法排课");
    }

    QHash<QString, Course> courseById;
    for (const auto &course : courses) {
        courseById.insert(course.id, course);
    }

    QSet<QString> remaining = ids;
    QSet<QString> completed;
    auto dependents = dependentsMap(courses);
    auto remainingCredits = [&](const QSet<QString> &rem) {
        int sum = 0;
        for (const auto &id : rem) sum += courseById[id].credits;
        return sum;
    };

    ScheduleResult result;
    for (int semester = 1; semester <= semesterCount; ++semester) {
        QVector<Course> available;
        for (const auto &id : remaining) {
            const auto &course = courseById[id];
            if (prereqsDone(course, completed)) {
                available.push_back(course);
            }
        }

        std::sort(available.begin(), available.end(), [&](const Course &a, const Course &b) {
            const int depA = dependents.value(a.id).size();
            const int depB = dependents.value(b.id).size();
            if (strategy == SchedulingStrategy::Frontload) {
                if (depA != depB) return depA > depB; // more dependents first
                if (a.credits != b.credits) return a.credits > b.credits; // higher credits first
            } else { // BalanceLoad
                if (a.credits != b.credits) return a.credits < b.credits; // smaller courses first
                if (depA != depB) return depA > depB; // still prefer more dependents when ties
            }
            return a.id < b.id; // stable tie-breaker
        });

        int used = 0;
        SemesterPlan plan;
        plan.index = semester;
        const int semestersLeft = semesterCount - semester + 1;
        const int totalRemCredits = remainingCredits(remaining);
        const int target = strategy == SchedulingStrategy::BalanceLoad
                               ? std::min(creditLimit,
                                          (totalRemCredits + semestersLeft - 1) / semestersLeft)
                               : creditLimit;

        for (const auto &course : available) {
            if (used + course.credits <= creditLimit) {
                plan.courses.append(course);
                used += course.credits;
                if (strategy == SchedulingStrategy::BalanceLoad && used >= target) {
                    break;
                }
            }
        }

        if (plan.courses.isEmpty() && !remaining.isEmpty()) {
            throw SchedulerError(QStringLiteral("在第 %1 学期无法安排任何课程，请检查学分上限或先修关系").arg(semester));
        }

        for (const auto &course : plan.courses) {
            remaining.remove(course.id);
            completed.insert(course.id);
        }

        result.semesters.append(plan);
    }

    if (!remaining.isEmpty()) {
        throw SchedulerError(QStringLiteral("在限定的 %1 个学期内无法完成所有课程：剩余 %2 门")
                                 .arg(semesterCount)
                                 .arg(remaining.size()));
    }

    return result;
}

bool Scheduler::hasCycle(const QList<Course> &courses) const {
    QHash<QString, QSet<QString>> graph;
    for (const auto &course : courses) {
        graph.insert(course.id, QSet<QString>(course.prerequisites.begin(), course.prerequisites.end()));
    }

    QSet<QString> visiting;
    QSet<QString> visited;

    std::function<bool(const QString &)> dfs = [&](const QString &node) -> bool {
        if (visiting.contains(node)) return true;
        if (visited.contains(node)) return false;
        visiting.insert(node);
        for (const auto &pre : graph.value(node)) {
            if (dfs(pre)) return true;
        }
        visiting.remove(node);
        visited.insert(node);
        return false;
    };

    for (const auto &id : graph.keys()) {
        if (dfs(id)) return true;
    }
    return false;
}

bool Scheduler::prereqsDone(const Course &course, const QSet<QString> &completed) const {
    for (const auto &pre : course.prerequisites) {
        if (!completed.contains(pre)) return false;
    }
    return true;
}

QHash<QString, QSet<QString>> Scheduler::dependentsMap(const QList<Course> &courses) const {
    QHash<QString, QSet<QString>> map;
    for (const auto &course : courses) {
        map[course.id]; // ensure exists
    }
    for (const auto &course : courses) {
        for (const auto &pre : course.prerequisites) {
            map[pre].insert(course.id);
        }
    }
    return map;
}
