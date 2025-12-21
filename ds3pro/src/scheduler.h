#pragma once

#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include <QVector>

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

class SchedulerError {
public:
    explicit SchedulerError(QString message) : m_message(std::move(message)) {}
    QString message() const { return m_message; }

private:
    QString m_message;
};

class Scheduler {
public:
    ScheduleResult schedule(int semesterCount, int creditLimit, const QList<Course> &courses) const;

private:
    bool hasCycle(const QList<Course> &courses) const;
    bool prereqsDone(const Course &course, const QSet<QString> &completed) const;
    QHash<QString, QSet<QString>> dependentsMap(const QList<Course> &courses) const;
};
