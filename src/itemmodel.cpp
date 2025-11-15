#include "itemmodel.h"

#include <QLocale>

ItemModel::ItemModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

int ItemModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 5;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return QVariant();
    }

    const InventoryItem &item = m_items.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item.name;
        case 1:
            return item.category;
        case 2:
            return item.quantity;
        case 3:
            return QLocale().toCurrencyString(item.unitPrice);
        case 4:
            return item.purchaseDate.toString(Qt::ISODate);
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == 2 || index.column() == 3) {
            return Qt::AlignRight | Qt::AlignVCenter;
        }
    }

    return QVariant();
}

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("名称");
        case 1:
            return tr("类别");
        case 2:
            return tr("数量");
        case 3:
            return tr("单价");
        case 4:
            return tr("采购日期");
        }
    }

    return section + 1;
}

Qt::ItemFlags ItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool ItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || count <= 0 || row + count > m_items.size()) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_items.removeAt(row);
    }
    endRemoveRows();
    return true;
}

void ItemModel::addItem(const InventoryItem &item)
{
    const int row = m_items.size();
    beginInsertRows(QModelIndex(), row, row);
    m_items.append(item);
    endInsertRows();
}

void ItemModel::clear()
{
    if (m_items.isEmpty()) {
        return;
    }
    beginResetModel();
    m_items.clear();
    endResetModel();
}

double ItemModel::totalCost() const
{
    double total = 0.0;
    for (const auto &item : m_items) {
        total += item.unitPrice * item.quantity;
    }
    return total;
}
