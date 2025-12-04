#include "mainwindow.h"
#include "expressionevaluator.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QLocale>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::openExpressionFile);
    connect(ui->evaluateButton, &QPushButton::clicked, this, &MainWindow::evaluateExpression);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clearAll);

    ui->treeWidget->setColumnCount(1);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->setAlternatingRowColors(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openExpressionFile()
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                     tr("选择表达式文件"),
                                                     QString(),
                                                     tr("文本文件 (*.txt);;所有文件 (*)"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("无法打开文件"), file.errorString());
        return;
    }

    const QString content = QString::fromUtf8(file.readAll());
    QString expression;
    QStringList assignmentLines;
    const QStringList lines = content.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::KeepEmptyParts);
    for (const QString &rawLine : lines) {
        const QString trimmed = rawLine.trimmed();
        if (expression.isEmpty() && !trimmed.isEmpty()) {
            expression = trimmed;
        } else {
            assignmentLines.append(rawLine);
        }
    }

    ui->expressionLineEdit->setText(expression);
    ui->assignmentEdit->setPlainText(assignmentLines.join(QStringLiteral("\n")));
    statusBar()->showMessage(tr("已从文件加载表达式。"), 3000);
}

void MainWindow::evaluateExpression()
{
    const QString expression = ui->expressionLineEdit->text().trimmed();
    if (expression.isEmpty()) {
        QMessageBox::information(this, tr("缺少表达式"), tr("请输入需要求值的表达式。"));
        ui->expressionLineEdit->setFocus();
        return;
    }

    QString parseError;
    if (!m_evaluator.ReadInput(expression, &parseError)) {
        QMessageBox::critical(this, tr("解析失败"), parseError);
        return;
    }

    QString assignmentError;
    if (!applyAssignments(&assignmentError)) {
        QMessageBox::critical(this, tr("赋值失败"), assignmentError);
        return;
    }

    double resultValue = 0.0;
    QString evaluateError;
    if (!m_evaluator.Evaluate(&resultValue, &evaluateError)) {
        QMessageBox::critical(this, tr("求值失败"), evaluateError);
        return;
    }

    ui->resultLabel->setText(tr("结果：%1").arg(QLocale().toString(resultValue)));
    populateTree();
    statusBar()->showMessage(tr("求值成功。"), 3000);
}

void MainWindow::clearAll()
{
    ui->expressionLineEdit->clear();
    ui->assignmentEdit->clear();
    ui->resultLabel->setText(tr("结果：--"));
    ui->treeWidget->clear();
    m_evaluator = ExpressionEvaluator();
    statusBar()->clearMessage();
}

void MainWindow::populateTree()
{
    ui->treeWidget->clear();
    const ExpressionEvaluator::Node *rootNode = m_evaluator.root();
    if (!rootNode) {
        return;
    }

    QTreeWidgetItem *rootItem = createTreeItem(rootNode);
    ui->treeWidget->addTopLevelItem(rootItem);
    ui->treeWidget->expandAll();
}

QTreeWidgetItem *MainWindow::createTreeItem(const ExpressionEvaluator::Node *node, QTreeWidgetItem *parent)
{
    if (!node) {
        return nullptr;
    }

    QString text = node->text;
    if (text.isEmpty() && node->type == ExpressionEvaluator::Node::Type::Number) {
        text = QLocale().toString(node->value);
    }

    auto *item = new QTreeWidgetItem();
    item->setText(0, text);

    if (node->left) {
        if (QTreeWidgetItem *child = createTreeItem(node->left.get(), item)) {
            item->addChild(child);
        }
    }
    if (node->right) {
        if (QTreeWidgetItem *child = createTreeItem(node->right.get(), item)) {
            item->addChild(child);
        }
    }

    if (!parent) {
        item->setExpanded(true);
    }

    return item;
}

bool MainWindow::applyAssignments(QString *errorMessage)
{
    const QString assignments = ui->assignmentEdit->toPlainText();
    return m_evaluator.Assign(assignments, errorMessage);
}
