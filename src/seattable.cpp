#include "seattable.hpp"

#include <QAbstractItemModel>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QModelIndex>
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
    if (!item || !model()) {
        return;
    }

    // 记录拖拽源。dropEvent 里不能只依赖 currentItem()，
    // 某些情况下 current 会随拖拽移动/选中变化。
    m_dragRow = item->row();
    m_dragCol = item->column();

    // 必须使用模型自己的 MIME 数据（application/x-qabstractitemmodeldatalist），
    // 否则 QTableWidget 的 canDropMimeData() 会拒绝普通文本，dropEvent 根本不会触发。
    QModelIndexList indexes;
    indexes.append(currentIndex());
    QMimeData* mime = model()->mimeData(indexes);
    if (!mime) {
        m_dragRow = -1;
        m_dragCol = -1;
        return;
    }

    // 自己启动拖拽，不走 QAbstractItemView::startDrag：
    // 后者在 InternalMove 且 drop 返回 MoveAction 时会调用 clearOrRemove()，
    // 可能把源单元格（甚至整行）清掉。真正的交换由 dropEvent 发信号完成。
    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
    delete drag;

    m_dragRow = -1;
    m_dragCol = -1;
}

void SeatTable::dragEnterEvent(QDragEnterEvent* event)
{
    // 只接受本表内部的拖拽，其他来源一律忽略
    if (event->source() == this) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void SeatTable::dragMoveEvent(QDragMoveEvent* event)
{
    // 只允许落在某个单元格上；绕过基类的 canDropMimeData 判断
    if (event->source() == this && itemAt(event->position().toPoint())) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void SeatTable::dropEvent(QDropEvent* event)
{
    auto resetDrag = [this]() {
        m_dragRow = -1;
        m_dragCol = -1;
    };

    // 优先使用 startDrag() 记录的源单元格
    int fromRow = m_dragRow;
    int fromCol = m_dragCol;
    if (fromRow < 0 || fromCol < 0) {
        QTableWidgetItem* source = currentItem();
        if (!source) {
            resetDrag();
            event->ignore();
            return;
        }
        fromRow = source->row();
        fromCol = source->column();
    }

    // 目标：鼠标落点下的单元格。itemAt 使用 viewport 坐标
    QTableWidgetItem* target = itemAt(event->position().toPoint());
    if (!target) {
        // 落在表格外或非法位置，忽略
        resetDrag();
        event->ignore();
        return;
    }

    const int toRow = target->row();
    const int toCol = target->column();

    // 拖到自己身上：忽略
    if (fromRow == toRow && fromCol == toCol) {
        resetDrag();
        event->ignore();
        return;
    }

    // 不调用 QTableWidget::dropEvent，只发信号让主窗口交换数据
    emit swapRequested(fromRow, fromCol, toRow, toCol);

    event->setDropAction(Qt::MoveAction);
    event->accept();
    resetDrag();

    // 替代基类 dropEvent 的收尾工作，避免视图停留在 DraggingState
    setState(NoState);
    viewport()->update();
}
