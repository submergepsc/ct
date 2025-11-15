#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QAbstractTableModel>
#include <QDate>
#include <QVector>

struct InventoryItem
{
    QString name;
    QString category;
    int quantity = 0;
    double unitPrice = 0.0;
    QDate purchaseDate;
};

class ItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void addItem(const InventoryItem &item);
    void clear();
    double totalCost() const;

private:
    QVector<InventoryItem> m_items;
};

#endif // ITEMMODEL_H
