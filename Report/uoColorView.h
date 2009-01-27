/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOCOLORVIEW_H
#define UOCOLORVIEW_H

#include <QAbstractItemView>

namespace uoReport {
/**
	Сласс для отображения сетки цветов и выбора цвета соответственно.
	планирую сделать как у 1С -ного редактора, но немножко навороченным:
	<pre>
	Выглядит приблизительно так:
	[*************v]< комбо-бокс
	[Авто          ]
	[][][][][][][][]
	[][][][][][][][]
	[][][][][][][][]
	[][][][][][][][]
	[][][][][][][][]
	[Дополнительно ]
	[][][][][][][][]
	[][][][]
	</pre>
	Попробую использовать модель из 2-х столбцов. Содержимое столбцов:
	1 - собственно цвет.
	2 - собственно числа: 0 - предзаданный; 1 - дополнительный.
	Дополнительно: первым значением в модели должен быть дефолтный цвет.
*/

class uoColorView : public QAbstractItemView
{
	Q_OBJECT
	public:
		uoColorView(QWidget *parent = 0);
		virtual ~uoColorView();

		virtual QRect visualRect(const QModelIndex &index) const;
		virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
		virtual QModelIndex indexAt(const QPoint &point) const;

	protected:
		virtual QModelIndex moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers);
		int horizontalOffset() const;
		int verticalOffset() const;

	    virtual bool isIndexHidden(const QModelIndex &index) const;
		virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
		virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;

	private:
		int m_itemSize;		//< размер квадратика в пикселах.
		int m_itemCountDef;	//< количество цветов дефолтных
		int m_itemCountUsr;	//< количество цветов пользвателя
		int m_itemInRow;
		int m_colCount;		//< количество столбцов во вьюве.

	protected slots:
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsInserted(const QModelIndex &parent, int start, int end);
		void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);


};

} // namespace uoReport

#endif // UOCOLORVIEW_H
