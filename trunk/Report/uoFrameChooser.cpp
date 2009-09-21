/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoFrameChooser.h"
#include <QPainter>
#include <QStyleOptionComboBox>
#include <QStyleOptionComplex>
#include <QStylePainter>
#include <QRect>
#include <QLine>
#include <QKeyEvent>

namespace uoReport {

uoFrameChooser::uoFrameChooser(QWidget *parent)
	:QWidget(parent)

{
	setMouseTracking(true); 	/// начинаем ловить перемещение мышки....
	setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_curentLine = -1;
    m_leftOffset = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    m_leftOffset = qMax(1,m_leftOffset);
    m_leftOffset =+2;
    fillLines();
}

uoFrameChooser::~uoFrameChooser()
{
	uoFC_LineItem* item = NULL;

	for(int i=0; i<=FRAMECHOOSER_LINEITEM_COUNT; i++){
		item = m_liArray[i];
		delete item;
		item = NULL;
	}
}

Qt::PenStyle uoFrameChooser::penStyle()
{
	if (m_curentLine == -1)
		return Qt::NoPen;
	return m_liArray[m_curentLine]->m_lineType;
}
void uoFrameChooser::setPenStyle(Qt::PenStyle style)
{
	if (style != penStyle()) {
		m_curentLine = -1;
		uoFC_LineItem* item = NULL;
		for(int i=0; i<=FRAMECHOOSER_LINEITEM_COUNT; i++){
			item = m_liArray[i];
			if (m_liArray[i]->m_lineType == style){
				m_curentLine = i;
				break;
			}
		}
		if (isVisible())
			emit update();
	}

}

uoCellBorderType  uoFrameChooser::borderType()
{
	if (m_curentLine != -1)
		return (uoCellBorderType) penStyle();
	return uoCBT_Unknown;
}
void  uoFrameChooser::setBorderType(uoCellBorderType style)
{
	int old = m_curentLine;
	if (style == uoCBT_Unknown)
		m_curentLine = -1;
	else
		setPenStyle((Qt::PenStyle)style);
	if (old != m_curentLine)
		emit update();
}



void uoFrameChooser::fillLines()
{
	m_liArray[0] = new uoFC_LineItem;	m_liArray[0]->m_lineType = Qt::NoPen;
	m_liArray[1] = new uoFC_LineItem;	m_liArray[1]->m_lineType = Qt::SolidLine;
	m_liArray[2] = new uoFC_LineItem;	m_liArray[2]->m_lineType = Qt::DashLine;
	m_liArray[3] = new uoFC_LineItem;	m_liArray[3]->m_lineType = Qt::DotLine;
	m_liArray[4] = new uoFC_LineItem;	m_liArray[4]->m_lineType = Qt::DashDotLine;
	m_liArray[5] = new uoFC_LineItem;	m_liArray[5]->m_lineType = Qt::DashDotDotLine;
	m_liArray[6] = new uoFC_LineItem;	m_liArray[6]->m_lineType = Qt::CustomDashLine;
	int i;
	for(i=0; i<=FRAMECHOOSER_LINEITEM_COUNT; i++){
		m_liArray[i]->m_offsetH = i * FRAMECHOOSER_LINEITEM_WIDHT + m_leftOffset;
	}
}



QSize uoFrameChooser::sizeHint() const{
    QFontMetrics fm(font());
    int h = int(qMax(fm.height(), 7)*1.6);
    int w = FRAMECHOOSER_LINEITEM_WIDHT * (FRAMECHOOSER_LINEITEM_COUNT+1) + m_leftOffset*6; // "some"
	return QSize(w,h);
}
QSize uoFrameChooser::minimumSizeHint() const{
	return sizeHint();
}

void uoFrameChooser::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(+0.5, +0.5);

   	const QPalette palette_c = palette();

    painter.setPen(palette().color(QPalette::Text));
    // draw the combobox frame, focusrect and selected etc.


	QPen frmCurentPen(palette().color(QPalette::Highlight));
	frmCurentPen.setStyle(Qt::DotLine);
	QPen curentLinePen(palette().color(QPalette::Text));

    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.frame = 1;
    opt.subControls = QStyle::SC_ComboBoxFrame;
	opt.state |= QStyle::State_Sunken;

    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

   	QRect frmRect = rect();
	QLine line;
	QPoint pt1, pt2;
	pt1.setY(frmRect.height()/2);
	pt2.setY(frmRect.height()/2);

	frmRect.adjust(m_leftOffset,m_leftOffset,-m_leftOffset,-m_leftOffset);

	if (m_curentLine == -1)	{
		painter.fillRect(frmRect, palette_c.color(QPalette::Window));
		frmCurentPen.setBrush(palette().color(QPalette::Highlight));
	}
	if (hasFocus()) {
		painter.setPen(frmCurentPen);
		painter.drawRect(frmRect);
	}
	frmRect.adjust(m_leftOffset,m_leftOffset,-m_leftOffset,-m_leftOffset);

	//m_leftOffset
	int offTop = m_leftOffset;
	offTop *= 2;
	int penWidht = qMax(1, curentLinePen.width());

	uoFC_LineItem* item = NULL;
    int i;
   	for(i=0; i<=FRAMECHOOSER_LINEITEM_COUNT; i++){
		//m_liArray[i]->m_offsetH = i * FRAMECHOOSER_LINEITEM_WIDHT + m_leftOffset;
		item = m_liArray[i];

		frmRect.setX(item->m_offsetH + offTop);
		frmRect.setRight(item->m_offsetH + FRAMECHOOSER_LINEITEM_WIDHT + offTop);
		if (m_curentLine == i)
		{
			// прорисуем рамку
			painter.setPen(frmCurentPen);
			painter.drawRect(frmRect);

		}
		curentLinePen.setStyle(item->m_lineType);
		painter.setPen(curentLinePen);
		pt1.setX(frmRect.x()+m_leftOffset*2);
		pt2.setX(frmRect.right()-m_leftOffset);

		line = QLine(pt1,pt2);
		if (item->m_lineType == Qt::CustomDashLine)	{
			line.translate(0,-penWidht);
			painter.drawLine(line);
			line.translate(0,2*penWidht);
			painter.drawLine(line);
		} else {
			painter.drawLine(line);
		}
	}
	item = NULL;

}

void uoFrameChooser::keyPressEvent ( QKeyEvent * event )
{
	int key = event->key();
	switch (key)	{
		case Qt::Key_Down:
		case Qt::Key_Right:
		{
			m_curentLine = m_curentLine + 1;
			if (m_curentLine > FRAMECHOOSER_LINEITEM_COUNT)
				m_curentLine = 0;
			emit frameChanged(m_curentLine);
			break;
		}
		case Qt::Key_Up:
		case Qt::Key_Left:
		{
			m_curentLine = m_curentLine - 1;
			if (m_curentLine < 0) {
				m_curentLine = FRAMECHOOSER_LINEITEM_COUNT;
			}
			emit frameChanged(m_curentLine);
			break;
		}
		default: {
			QWidget::keyPressEvent( event );
			return;
		}
	}
	update();

}
void uoFrameChooser::mousePressEvent(QMouseEvent *event)
{
	QPoint pt = event->pos();
	event->accept();
   	QRect frmRect = rect();
	frmRect.adjust(m_leftOffset,m_leftOffset,-m_leftOffset,-m_leftOffset);

	int i = 0, pos = -1, offTop = m_leftOffset;
	offTop += 2;
	uoFC_LineItem* item = NULL;
    for(i=0; i<=FRAMECHOOSER_LINEITEM_COUNT; i++){
		item = m_liArray[i];
		frmRect.setX(item->m_offsetH + offTop);
		frmRect.setRight(item->m_offsetH + FRAMECHOOSER_LINEITEM_WIDHT + offTop);
		if (frmRect.contains(pt)){
			pos = i;
			break;
		}
	}
	if (pos != -1){
		m_curentLine = pos;
		emit frameChanged(m_curentLine);
		update();
	}
}




void uoFrameChooser::changeEvent ( QEvent * event )
{
	if (event->type() == QEvent::StyleChange)
	{
		m_leftOffset = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
		m_leftOffset =+2;
	}
	QWidget::changeEvent ( event );
}

} //namespace uoReport
