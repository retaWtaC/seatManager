#pragma once

#include <QTableWidget>

class QDropEvent;

// 座位表：只处理“单元格 -> 单元格”的拖拽交换。
// 不调用基类 dropEvent，避免 QTableWidget 默认的“移动/覆盖”行为。
class SeatTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit SeatTable(int rows, int columns, QWidget* parent = nullptr);

signals:
    // 拖拽源与目标均使用 0 基行列号
    void swapRequested(int fromRow, int fromCol, int toRow, int toCol);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dropEvent(QDropEvent* event) override;
};
