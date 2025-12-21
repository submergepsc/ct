#pragma once

#include <QList>
#include <QHash>
#include <QVector>
#include <QSet>
#include <QString>
#include <QStringList>

struct Course {
    QString id;
    QString name;
    int credits{};
    QStringList prerequisites;
};

struct SemesterPlan {
    int index{};
    QList<Course> courses;
    int totalCredits() const;
};

struct ScheduleResult {
    QList<SemesterPlan> semesters;
    int totalCredits() const;
    int totalCourses() const;
};

enum class SchedulingStrategy {
    BalanceLoad,
    Frontload
};

class SchedulerError {
public:
    explicit SchedulerError(QString message) : m_message(std::move(message)) {}
    QString message() const { return m_message; }

private:
    QString m_message;
};

class Scheduler {
public:
    ScheduleResult schedule(int semesterCount,
                            int creditLimit,
                            const QList<Course> &courses,
                            SchedulingStrategy strategy) const;

private:
    bool hasCycle(const QList<Course> &courses) const;
    bool prereqsDone(const Course &course, const QSet<QString> &completed) const;
    QHash<QString, QSet<QString>> dependentsMap(const QList<Course> &courses) const;
};
