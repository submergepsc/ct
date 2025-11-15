#include "mainwindow.h"
#include "itemmodel.h"
#include "ui_mainwindow.h"

#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QLocale>
#include <QList>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new ItemModel(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->purchaseDateEdit->setDate(QDate::currentDate());
    connectSignals();
    updateSummary();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSignals()
{
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::addItem);
    connect(ui->removeButton, &QPushButton::clicked, this, &MainWindow::removeSelected);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clearItems);
    connect(m_model, &QAbstractItemModel::dataChanged, this, &MainWindow::updateSummary);
    connect(m_model, &QAbstractItemModel::modelReset, this, &MainWindow::updateSummary);
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &MainWindow::updateSummary);
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &MainWindow::updateSummary);
}

void MainWindow::resetForm()
{
    ui->nameLineEdit->clear();
    ui->categoryComboBox->setCurrentIndex(0);
    ui->quantitySpinBox->setValue(1);
    ui->priceDoubleSpinBox->setValue(0.0);
    ui->purchaseDateEdit->setDate(QDate::currentDate());
    ui->nameLineEdit->setFocus();
}

void MainWindow::addItem()
{
    const QString name = ui->nameLineEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("缺少名称"), tr("请填写物品名称。"));
        ui->nameLineEdit->setFocus();
        return;
    }

    InventoryItem item;
    item.name = name;
    item.category = ui->categoryComboBox->currentText();
    item.quantity = ui->quantitySpinBox->value();
    item.unitPrice = ui->priceDoubleSpinBox->value();
    item.purchaseDate = ui->purchaseDateEdit->date();

    m_model->addItem(item);
    resetForm();
}

void MainWindow::removeSelected()
{
    const auto selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("未选择"), tr("请选择要删除的记录。"));
        return;
    }

    // remove from bottom to top to keep indexes valid
    QList<int> rows;
    rows.reserve(selection.size());
    for (const QModelIndex &index : selection) {
        rows.append(index.row());
    }
    std::sort(rows.begin(), rows.end());

    for (int i = rows.count() - 1; i >= 0; --i) {
        m_model->removeRow(rows.at(i));
    }
}

void MainWindow::clearItems()
{
    if (m_model->rowCount() == 0) {
        return;
    }

    const auto reply = QMessageBox::question(this, tr("清空列表"), tr("确定要清空所有记录吗？"));
    if (reply == QMessageBox::Yes) {
        m_model->clear();
    }
}

void MainWindow::updateSummary()
{
    const int count = m_model->rowCount();
    const double total = m_model->totalCost();
    const QString text = tr("共 %1 项，合计：%2")
                             .arg(count)
                             .arg(QLocale().toCurrencyString(total));
    ui->summaryLabel->setText(text);
}
