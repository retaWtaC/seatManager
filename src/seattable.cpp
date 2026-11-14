#include "seattable.hpp"

#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QTableWidgetItem>

SeatTable::SeatTable(int rows, int columns, QWidget* parent)
    : QTableWidget(rows, columns, parent)
{
    // 使用 Qt 内部的拖拽机制负责开始拖拽/指示器，落点由 dropEvent 接管
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void SeatTable::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    QTableWidgetItem* item = currentItem();
    if (!item) {
        return;
    }

    // 自己启动拖拽，而不是走 QAbstractItemView::startDrag：
    // 后者在 InternalMove 且 drop 返回 MoveAction 时会调用 clearOrRemove()，
    // 可能把源单元格（甚至整行）清掉。这里真正的交换由 dropEvent 发信号完成，
    // 拖拽本身只需要一个 MIME 数据来启动。
    auto* mime = new QMimeData;
    mime->setText(item->text());

    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
    delete drag;
}

void SeatTable::dropEvent(QDropEvent* event)
{
    // 拖动源：当前项（开始拖拽时由视图设置为 current）
    QTableWidgetItem* source = currentItem();
    if (!source) {
        event->ignore();
        return;
    }

    // 目标：鼠标落点下的单元格。itemAt 使用 viewport 坐标
    QTableWidgetItem* target = itemAt(event->position().toPoint());
    if (!target) {
        // 落在表格外或非法位置，忽略
        event->ignore();
        return;
    }

    const int fromRow = source->row();
    const int fromCol = source->column();
    const int toRow = target->row();
    const int toCol = target->column();

    // 拖到自己身上：忽略
    if (fromRow == toRow && fromCol == toCol) {
        event->ignore();
        return;
    }

    // 不调用 QTableWidget::dropEvent，只发信号让主窗口交换数据
    emit swapRequested(fromRow, fromCol, toRow, toCol);
    event->acceptProposedAction();

    // 替代基类 dropEvent 的收尾工作，避免视图停留在 DraggingState
    setState(NoState);
    viewport()->update();
}
