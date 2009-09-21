/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoColorChooser.h"
#include "uoColorCContainer.h"
#include <QPainter>
#include <QBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QStylePainter>
#include <QKeyEvent>
#include <QDebug>
#include <QPalette>
#include <QRect>
#include <QStyleOption>

/**
	\class uoColorChooser - Контрол для выбора цвета.
	\brief Контрол для выбора цвета. Не доработан, но дело свое делает :)
	Класс для отображения сетки цветов и выбора цвета соответственно.
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
	Дополнительно: первым значением в модели должен быть дефолтный цвет.
*/


uoColorChooser::uoColorChooser(QWidget *parent)
	: QFrame(parent)
{
	setMouseTracking(true); 	/// начинаем ловить перемещение мышки....
	setFocusPolicy(Qt::StrongFocus);
	m_allAddBtnSize = 0;
	m_curCtrl = 0;
	m_addBtn = 0;
	m_itemInRow = 8;
	m_colorArea = QRect(0,0,0,0);

    setLineWidth(0);
    parent->installEventFilter(this);

    setFrameStyle(Sunken);
    setFrameShape(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    fillColors();
    m_Container = NULL;
    getContainer();
    setupBotton();
    m_colorCurPos = 0;

}

uoColorCContainer*
uoColorChooser::getContainer()
{
	if (!m_Container){
		m_Container = new uoColorCContainer(this);
		if (m_Container){
		}
	}
	return m_Container;
}


uoColorChooser::~uoColorChooser()
{
	//dtor
	delete m_Container;
}
int uoColorChooser::addColor(QColor col)
{
	m_colorList.append(QColor(col));
	if (!m_colorCur.isValid()) {
		m_colorCur = col;
		m_colorCurPos = m_colorList.count()-1;
	}
	return m_colorList.count()-1;
}

QColor uoColorChooser::getColor(int pos)
{
	if (pos != -1) {
		if (pos<m_colorList.size())
			return m_colorList.at(pos);
	}
	return m_colorCur;
}


void uoColorChooser::fillColors(){

	m_colorList.append(QColor("#000000"));
	m_colorList.append(QColor("#ffffff"));
	m_colorList.append(QColor("#ff0000"));
	m_colorList.append(QColor("#00ff00"));
	m_colorList.append(QColor("#0000ff"));//blue
	m_colorList.append(QColor("#ffff00"));// желтый
	m_colorList.append(QColor("#ff00ff"));	//
	m_colorList.append(QColor("#00ffff"));	//

	m_colorList.append(QColor("#800000"));	//
	m_colorList.append(QColor("#008000"));	//
	m_colorList.append(QColor("#808000"));	//
	m_colorList.append(QColor("#000080"));	//
	m_colorList.append(QColor("#800080"));	//
	m_colorList.append(QColor("#008080"));	//
	m_colorList.append(QColor("#808080"));	//
	m_colorList.append(QColor("#c0c0c0"));	//

	//3
	m_colorList.append(QColor("#8080ff"));	//
	m_colorList.append(QColor("#802060"));	//
	m_colorList.append(QColor("#ffffc0"));	//
	m_colorList.append(QColor("#a0e0e0"));	//
	m_colorList.append(QColor("#600080"));	//
	m_colorList.append(QColor("#ff8080"));	//
	m_colorList.append(QColor("#0080c0"));	//
	m_colorList.append(QColor("#c0c0ff"));	//

	//4
	m_colorList.append(QColor("#00cfff"));	//
	m_colorList.append(QColor("#69ffff"));	//
	m_colorList.append(QColor("#e0ffe0"));	//
	m_colorList.append(QColor("#dd9cb3"));	//
	m_colorList.append(QColor("#b38fee"));	//
	m_colorList.append(QColor("#2a6ff9"));	//
	m_colorList.append(QColor("#3fb8cd"));	//
	m_colorList.append(QColor("#488436"));	//

	// 5
	m_colorList.append(QColor("#958c41"));	//
	m_colorList.append(QColor("#905e42"));	//
	m_colorList.append(QColor("#a0627a"));	//
	m_colorList.append(QColor("#624fac"));	//
	m_colorList.append(QColor("#1d2fbe"));	//
	m_colorList.append(QColor("#286676"));	//
	m_colorList.append(QColor("#004500"));	//
	m_colorList.append(QColor("#453e01"));	//

	// 6
	m_colorList.append(QColor("#6a2813"));	//
	m_colorList.append(QColor("#85396a"));	//
	m_colorList.append(QColor("#4a3285"));	//
	m_colorList.append(QColor("#c0dcc0"));	//
	m_colorList.append(QColor("#a6caf0"));	//
	m_colorList.append(QColor("#7fffd4"));	//
	m_colorList.append(QColor("#f0ffff"));	//
	m_colorList.append(QColor("#f5f5dc"));	///// Повтор?!
	// Повторы?!
	// 7
	m_colorList.append(QColor("#808000"));	//
	m_colorList.append(QColor("#800080"));	//
	m_colorList.append(QColor("#008080"));	//
	m_colorList.append(QColor("#808080"));	//
	m_colorList.append(QColor("#fffbf0"));	//
	m_colorList.append(QColor("#a0a0a4"));	//
	m_colorList.append(QColor("#313900"));	//
	m_colorList.append(QColor("#d9853e"));	//
}

QSize uoColorChooser::sizeHint() const{

    QFontMetrics fm(font());
    int h = int(qMax(fm.height(), 7)* 1.6);
    int w = int(fm.width(QLatin1Char('x')) * 7) + m_allAddBtnSize; // "some"
	return QSize(w,h);
}
QSize uoColorChooser::minimumSizeHint() const{
    QFontMetrics fm(font());
    int h = int(qMax(fm.height(), 7)*1.6);
    int w = int(fm.width(QLatin1Char('x')) * 7) + m_allAddBtnSize; // "some"
	return QSize(w,h);
}

int uoColorChooser::addButton(const QString& text, uoCC_Btn type)
{
	if (text.isEmpty())
		return 0;
	uoAddButton* btn = new uoAddButton;
	btn->m_text = text;
	btn->m_pressed = false;
	btn->m_Type 	= type;

	m_btns.append(btn);
	m_addBtn = m_btns.count();
	return m_addBtn;
}

int uoColorChooser::addButtonPrimitive(const QStyle::PrimitiveElement prim, uoCC_Btn type)
{
	if (!(
		prim == QStyle::PE_IndicatorArrowUp ||
		prim == QStyle::PE_IndicatorArrowDown||
		prim == QStyle::PE_IndicatorArrowRight||
		prim == QStyle::PE_IndicatorArrowLeft
	))
		return 0;
	uoAddButton* btn = new uoAddButton;
	btn->m_PE = prim;
	btn->m_pressed = false;
	btn->m_Type 	= type;
	m_btns.append(btn);
	m_addBtn = m_btns.count();
	return m_addBtn;
}


void uoColorChooser::recalcRects()
{
	m_allAddBtnSize = 0;
	if(m_btns.isEmpty())
		return;

	QRect frmRect = frameRect();
	int offTop = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

	if (!frmRect.isValid())
		return;
	QFontMetrics fm(font()); // Подрасчитаем размеры..
	frmRect.adjust(0,offTop,0,-offTop);

	uoAddButton* btn = NULL;
	QList<uoAddButton*>::iterator it = m_btns.begin();
	while(it != m_btns.end()){
		btn = *it;
		if (QStyle::PE_CustomBase == btn->m_PE) {
			btn->m_size = fm.width(btn->m_text)+4*offTop + 2;
		} else {
			btn->m_size = fm.width(QString("X"))+4*offTop + 2;
		}

		btn->m_rect = QRect(frmRect.topLeft(),frmRect.bottomLeft());
		btn->m_rect.adjust(0,0,btn->m_size,0);
		m_allAddBtnSize += 	btn->m_size;
		it++;
	}
	int endOf = frmRect.width()-(m_allAddBtnSize + offTop*m_btns.count());
	m_colorArea.setTopLeft(QPoint(offTop, offTop));
	m_colorArea.setBottomRight(QPoint(endOf, frmRect.bottom()));
	it = m_btns.begin();
	while(it != m_btns.end()){
		btn = *it;
		btn->m_rect.translate(endOf,0);
		endOf = endOf + btn->m_size + offTop;
		it++;
	}
}



void uoColorChooser::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
   	const QPalette palette_c = palette();

    painter.setPen(palette().color(QPalette::Text));
    // draw the combobox frame, focusrect and selected etc.

	int offTop = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

	QRect frmRect = frameRect();
//	QStyleOptionFrame opt;
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.frame = 1;
    opt.subControls = QStyle::SC_ComboBoxFrame;
	opt.state |= QStyle::State_Sunken;

    painter.drawComplexControl(QStyle::CC_ComboBox, opt);


	int endOf = m_allAddBtnSize + offTop*m_btns.count();

	frmRect.adjust(0,0,-endOf+offTop,0);
	QRect rectCbEdtFld = frmRect;
	opt.rect = frmRect;

//	opt.subControls = QStyle::SC_ComboBoxArrow;// | QStyle::SC_ComboBoxFrame;
//	opt.state |= QStyle::State_Sunken;
	// нужно отрисовать цвет.
//    if (m_curCtrl == 0)		opt.

//	painter.drawComplexControl(QStyle::CC_ComboBox, opt);
	if (!m_btns.isEmpty()){
		QList<uoAddButton*>::iterator it = m_btns.begin();
		uoAddButton* btn = NULL;
		int curBtn = 0;

		while(it != m_btns.end()){
			btn = *it;
			++curBtn;
			QStyleOptionButton optB;
			optB.initFrom(this);
			optB.features = QStyleOptionButton::None;

			optB.rect = btn->m_rect;
			if (btn->m_pressed){
				optB.state |= QStyle::State_Sunken;
			}
			optB.state |= QStyle::State_Raised;
			if (!(hasFocus() && m_curCtrl == curBtn)) {
				QStyle::State st = QStyle::State_HasFocus;
				st = ~st;
				optB.state = optB.state & st;
			}

			if (QStyle::PE_CustomBase == btn->m_PE) {
				optB.text = btn->m_text;
				painter.drawControl(QStyle::CE_PushButton, optB);
			} else {
				painter.drawControl(QStyle::CE_PushButton, optB);
				painter.drawPrimitive(btn->m_PE, optB);
			}
			it++;
		}
	}

	rectCbEdtFld.adjust(offTop,offTop,-offTop,-offTop); // рект готов.
	if (rectCbEdtFld.isValid()){
		painter.fillRect(rectCbEdtFld,palette_c.window());
		rectCbEdtFld.adjust(offTop,offTop,-offTop,-offTop); // рект готов.
		if (rectCbEdtFld.isValid()) {
			if (hasFocus() /*&& m_curCtrl == 0-*/) {
				painter.setPen(Qt::DashLine);
				painter.drawRect(rectCbEdtFld);
			}
			rectCbEdtFld.adjust(offTop,offTop,-offTop,-offTop); // рект готов.
			if (rectCbEdtFld.isValid()) {
				painter.fillRect(rectCbEdtFld,QBrush(m_colorCur));
			}
		}
	}
}

void uoColorChooser::resizeEvent ( QResizeEvent * event )
{
	QFrame::resizeEvent(event);
	recalcRects();
}

void uoColorChooser::setCurentColor(int colNum)
{	int colCount = m_colorList.count();
	if (colCount<colNum) {
		colNum = colNum - colCount;
	} else if (colNum<0){
		colNum = colNum + colCount;
	}
	if(colNum >= 0 && colNum<colCount)
	{
		m_colorCur = m_colorList.at(colNum);
		m_colorCurPos = colNum;
		update();
	}

}

void uoColorChooser::checkButtonAction(uoAddButton* btn)
{
	if (btn){
		bool vis = popupVisible();
		if (btn->m_Type == uoBtn_showPalette){
			if (vis) {
				popupHide();
			} else {
				popupShow();
			}
		} else {
			popupHide();
			if (btn->m_Type == uoBtn_colorClear){
				setCurentColor(0);
			} else if (btn->m_Type == uoBtn_colorChoose){
				//setCurentColor(0);
			}
		}
	}
}


void uoColorChooser::keyPressEvent ( QKeyEvent * event )
{
	int key = event->key();
	resetButton();
	uoAddButton* btn = NULL;
	switch (key)	{
		case Qt::Key_Down:
		case Qt::Key_Up:
		case Qt::Key_Right:
		case Qt::Key_Left:
		{
			if (!m_colorList.isEmpty()){
				int curColor = m_colorCurPos;
				switch (key)	{
				case Qt::Key_Down:
				case Qt::Key_Right: {curColor += 1; break;}

				case Qt::Key_Up: 	{curColor -= 1; break;}
				case Qt::Key_Left:	{curColor -= 1; break;}
				default: {break;}
				}
				setCurentColor(curColor);
				update();
			}
			break;
		}
		case Qt::Key_Escape:
		case Qt::Key_Enter:
		case Qt::Key_Return:
		{
			if (popupVisible()){
				event->accept();
				popupHide();
				return;
			} else {
				QFrame::keyPressEvent( event );

			}
			break;
		}


		case Qt::Key_Space:
		case Qt::Key_F4:
		{
			if (!popupVisible()) {
				popupShow();
			} else {
				popupHide();
			}
			break;
		}
		default: {
			QFrame::keyPressEvent( event );
			break;
		}
	}
	if (btn)
		checkButtonAction(btn);


}

void uoColorChooser::focusInEvent ( QFocusEvent * event )
{
	m_curCtrl = 0;
	resetButton();
	update();
}

void uoColorChooser::focusOutEvent ( QFocusEvent * event )
{
	resetButton();
	update();
}

QRect uoColorChooser::getGlobalRect()
{
	return QRect(mapToGlobal(frameGeometry().topLeft()),mapToGlobal(frameGeometry().bottomRight()));
}

void uoColorChooser::mousePressEvent(QMouseEvent *event)
{
	QPoint pt = event->pos();
	event->accept();
	resetButton();
	bool find = false;
	bool findInGenRect = false;
	if (m_colorArea.contains(pt))
		findInGenRect = true;


	m_curCtrl = 0;
	uoAddButton* btn = NULL;
	QList<uoAddButton*>::iterator it = m_btns.begin();
	while(it != m_btns.end()){
		btn = *it;
		++m_curCtrl;
		if (btn){
			if (findInGenRect && btn->m_Type == uoBtn_showPalette) {
				find = true;
				break;
			}

			if (btn->m_rect.contains(pt)) {
				btn->m_pressed = true;
				find = true;
				break;
			}
		}
		it++;
	}
	if (!find){
		m_curCtrl = 0;
	}
	update();
	if (find)
		checkButtonAction(btn);
}

void uoColorChooser::timerEvent(QTimerEvent* e)
{
	killTimer(e->timerId());
	if (popupVisible()){
		popupHide();
	}

}

void uoColorChooser::mouseReleaseEvent(QMouseEvent *event)
{
	event->accept();
	resetButton();
	update();
}

bool uoColorChooser::eventFilter ( QObject * watched, QEvent * event )
{
	switch(event->type()){
		case QEvent::MouseButtonPress:
		case QEvent::NonClientAreaMouseButtonPress:
		{
			if (popupVisible()){
				QMouseEvent *m = static_cast<QMouseEvent *>(event);

				if (!m_Container->rect().contains(m->pos())) {
					popupHide();
					return true;
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
	return false;
}

///// Надо попробовать поймать сигнал, который бы проинформировал о том, что надо спрятать попуп.
//bool uoColorChooser::event ( QEvent * event )
//{
//	QEvent::Type t = event->type();
//	switch(t){
//		case QEvent::Hide:
//		case QEvent::HideToParent:
//		case QEvent::WindowDeactivate:
//		{
//			startTimer(50 /*QApplication::doubleClickInterval()*/);
//			break;
//		}
//		default:{
//			break;
//		}
//	}
//	return QFrame::event(event);
//}

void uoColorChooser::hideEvent(QHideEvent *)
{
	popupHide();
}

void uoColorChooser::setupBotton()
{
	addButtonPrimitive(QStyle::PE_IndicatorArrowDown, uoBtn_showPalette);
	addButton("O", uoBtn_colorChoose);
/*
	addButton("X", uoBtn_colorClear);
*/
}


void uoColorChooser::popupShow()
{
	if (m_colorList.isEmpty())
		return;
	if (m_Container->isVisible()){
		popupHide();
		return;
	}

	getContainer();
	QSize sz = m_Container->recalcSize();
	m_Container->doChangeCurColor(m_colorCurPos);

    QPoint below = mapToGlobal(frameRect().bottomLeft());
    QRect rectScrin = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(this));
    QRect contRect = QRect(below, QPoint(below.x()+sz.width(), below.y()+sz.height()));

	contRect.moveTopLeft(below);
    if (!rectScrin.contains(contRect.bottomRight())){
		contRect.translate(0,-(frameRect().height()+contRect.height()));
    }
    m_Container->setGeometry(contRect);
	m_Container->raise();
    m_Container->show();
    m_Container->update();
    m_Container->setFocus();
}

void uoColorChooser::popupHide()
{
	getContainer();
    m_Container->hide();
}

bool uoColorChooser::popupVisible()
{

	if (m_Container){
		return m_Container->isVisible();
	}
	return false;
}


void uoColorChooser::resetButton()
{
	uoAddButton* btn = NULL;
	QList<uoAddButton*>::iterator it = m_btns.begin();
	while(it != m_btns.end()){
		btn = *it;
		if (btn)
			btn->m_pressed = false;
		it++;
	}
}

