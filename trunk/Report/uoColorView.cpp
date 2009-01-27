/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoColorView.h"

namespace uoReport {

uoColorView::uoColorView(QWidget *parent)
	: QAbstractItemView(parent)
{
	//ctor
	m_itemInRow = 8;
	m_itemSize = 16;
	m_itemCountDef = 0;
	m_itemCountUsr = 0;
}

uoColorView::~uoColorView()
{
	//dtor
}

/// Сигнал об изменении данных.
void uoColorView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QAbstractItemView::dataChanged(topLeft, bottomRight);
   	m_itemCountDef = model()->rowCount(rootIndex());
    viewport()->update();
}
void uoColorView::rowsInserted(const QModelIndex &parent, int start, int end){
   	m_itemCountDef = model()->rowCount(rootIndex());

}
void uoColorView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end){
   	m_itemCountDef = model()->rowCount(rootIndex());
}


/* Returns the rectangle on the viewport occupied by the item at index.
If your item is displayed in several areas then visualRect should
return the primary area that contains index and not the complete
area that index might encompasses, touch or cause drawing. */
QRect uoColorView::visualRect(const QModelIndex &index) const
{
	return QRect();
}

/*	Scrolls the view if necessary to ensure that the item at index is visible.
	The view will try to position the item according to the given hint. */
void uoColorView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
}

QModelIndex uoColorView::indexAt(const QPoint &point) const
{
	if (m_itemCountDef <= 0)
		return QModelIndex();

}


QModelIndex uoColorView::moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = currentIndex();

//    switch (cursorAction) {
//        case MoveLeft:
//        case MoveUp:
//            if (current.row() > 0)
//                current = model()->index(current.row() - 1, current.column(),
//                                         rootIndex());
//            else
//                current = model()->index(0, current.column(), rootIndex());
//            break;
//        case MoveRight:
//        case MoveDown:
//            if (current.row() < rows(current) - 1)
//                current = model()->index(current.row() + 1, current.column(),
//                                         rootIndex());
//            else
//                current = model()->index(rows(current) - 1, current.column(),
//                                         rootIndex());
//            break;
//        default:
//            break;
//    }

    viewport()->update();
    return current;

}

int uoColorView::horizontalOffset() const
{
	return 0;
}
int uoColorView::verticalOffset() const
{
	return 0;
}

bool uoColorView::isIndexHidden(const QModelIndex &index) const{
	return false;
}

void uoColorView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{}

QRegion uoColorView::visualRegionForSelection(const QItemSelection &selection) const
{
	return QRegion();
}




} //namespace uoReport
