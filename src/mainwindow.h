#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "expressionevaluator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void openExpressionFile();
    void evaluateExpression();
    void clearAll();

private:
    void populateTree();
    QTreeWidgetItem *createTreeItem(const ExpressionEvaluator::Node *node, QTreeWidgetItem *parent = nullptr);
    bool applyAssignments(QString *errorMessage);

    Ui::MainWindow *ui;
    ExpressionEvaluator m_evaluator;
};

#endif // MAINWINDOW_H
