/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOCOLORCCONTAINER_H
#define UOCOLORCCONTAINER_H
#include <QFrame>
#include <QWidget>
#include "uoColorChooser.h"


class uoColorChooser;

class uoColorCContainer : public QFrame
{
	Q_OBJECT

	public:
		uoColorCContainer(uoColorChooser* Chooser);
		QSize recalcSize();
		struct uoColRect
		{
			int m_colorNum;
			QRect m_rect;
		};
		int doChangeCurColor(int key);
    protected:
		void paintEvent(QPaintEvent *event);
		void keyPressEvent ( QKeyEvent * event );
        void mousePressEvent(QMouseEvent *event);

	private:
		QList<uoColRect> m_colRects;
		uoColorChooser* m_chooser;
		int m_gridSize;
		int m_gridMargin;
		int m_itemInRow;
		int m_curentItem;
		int m_itemCount;

		void setCurentItem(int curItem);

};

#endif // UOCOLORCCONTAINER_H
