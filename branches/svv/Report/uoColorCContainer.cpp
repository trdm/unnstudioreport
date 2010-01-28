/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoColorCContainer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtCore>
uoColorCContainer::uoColorCContainer(uoColorChooser* Chooser)
	:QFrame(Chooser,Qt::Popup) , m_chooser(Chooser)
{
    setMouseTracking(true);
	hide();

    setFocusPolicy(Qt::StrongFocus);
    setUpdatesEnabled(true);

	m_gridSize = 18;
	m_gridMargin = 2;
	m_itemInRow = 8;
	m_curentItem = 0;
	m_itemCount = m_chooser->m_colorList.count();
}

/// Пересчитаем размеры для вывода квадратиков цветов.
QSize uoColorCContainer::recalcSize()
{
	QSize size(0,0);
	m_colRects.clear();
	int colCount = m_chooser->m_colorList.count();
	if (colCount == 0)
		return size;
	int col=0, row=1, colMax=0;
	for(int i=0; i<colCount; i++){
		++col;
		if (col > m_itemInRow){
			col = 1;
			++row;
		}
		uoColRect cr;
		cr.m_colorNum = i;
		cr.m_rect = QRect(0,0,m_gridSize,m_gridSize);
		cr.m_rect.translate((col-1)*m_gridSize + m_gridMargin, (row-1)*m_gridSize + m_gridMargin);
		m_colRects.append(cr);
		colMax = qMax(colMax, col);

	}
	size.setHeight(row*m_gridSize + m_gridMargin*2);
	size.setWidth(colMax*m_gridSize + m_gridMargin*2);

	return size;
}

void uoColorCContainer::paintEvent(QPaintEvent *event)
{

	const QPalette palette_c = palette();
	QPalette::ColorGroup curColGrp = QPalette::Active;
	if (!isEnabled()) curColGrp = QPalette::Disabled;

	// нормальный фон окна.
   	QBrush brushWindow = palette_c.brush(QPalette::Window); //	brushWindow.setColor(palette_c.color(curColGrp, QPalette::Window));

	QPainter p(this);
	QRect rct = rect();
	p.drawRect(rct);
	rct.adjust(0,0,-1,-1);
	p.drawRect(rct);
	p.setPen(Qt::DashLine);


	if (m_colRects.isEmpty())
		return;
	QList<uoColRect>::const_iterator  cIt = m_colRects.begin();
	uoColRect colrRct;
	int cur = 0;
	while(cIt != m_colRects.end()){
		colrRct = *cIt;
		QRect rc = colrRct.m_rect;
		if (m_curentItem == cur)
			p.drawRect(rc);
		rc.adjust(m_gridMargin,m_gridMargin,-m_gridMargin,-m_gridMargin);
		QBrush br(m_chooser->getColor(cur));
		p.fillRect(rc, br);
		cIt++;
		++cur;
	}

}

void uoColorCContainer::mousePressEvent(QMouseEvent *event)
{
	QPoint pt = event->pos();
	QList<uoColRect>::const_iterator  cIt = m_colRects.begin();
	uoColRect colrRct;
	int cur = 0;
	while(cIt != m_colRects.end()){
		colrRct = *cIt;
		if (colrRct.m_rect.contains(pt))
		{
			m_curentItem = cur;
			cur = -1;
			break;
			event->accept();
		}
		cIt++;
		++cur;
	}
	if (cur == -1){
		setCurentItem(m_curentItem);
	} else {
		/* если мы собственно кликаем на uoColorChooser,
		что собственно должно закрыть попап, то закроем его,
		но если в другом месте, то нужно пропустить клик. */
		QRect rctParent = m_chooser->getGlobalRect();

		if (rctParent.contains(event->globalPos()))
			setAttribute(Qt::WA_NoMouseReplay);
	}
	hide();
}


void uoColorCContainer::keyPressEvent ( QKeyEvent * event )
{
	int key = event->key();
	bool procc = true;
	switch (key)	{
		case Qt::Key_Down:{
			m_curentItem += m_itemInRow;
			break;
		}
		case Qt::Key_Up:{
			m_curentItem -= m_itemInRow;
			break;
		}
		case Qt::Key_Right:{
			m_curentItem += 1;
			break;
		}
		case Qt::Key_Left:	{
			m_curentItem -= 1;
			break;
		}
		case Qt::Key_Escape:
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Space:
		case Qt::Key_F4:
		{
			event->accept();
			hide();
			m_chooser->setFocus();
			break;
		}
		default: {
			procc = false;
			break;
		}
	}
	if (procc) {
		if (m_curentItem>m_itemCount){
			m_curentItem -= m_itemCount;
		} else if (m_curentItem<0) {
			m_curentItem += m_itemCount;
		}
		setCurentItem(m_curentItem);
	}

}

void uoColorCContainer::setCurentItem(int curItem)
{
	update();
	m_chooser->setCurentColor(m_curentItem);
}


int uoColorCContainer::doChangeCurColor(int key)
{
	int old = m_curentItem;
	m_curentItem = key;
	update();
	return old;
}
