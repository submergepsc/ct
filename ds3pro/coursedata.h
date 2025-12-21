#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "scheduler.h"

class CourseIO {
public:
    static QList<Course> parse(const QString &text);
    static QString serialize(const QList<Course> &courses);

    static bool loadFile(const QString &path, QString &content, QString &error);
    static bool saveFile(const QString &path, const QString &content, QString &error);
    static bool writeIfMissing(const QString &path, const QList<Course> &courses, QString &error);

    static QList<Course> defaultCourses();
};
