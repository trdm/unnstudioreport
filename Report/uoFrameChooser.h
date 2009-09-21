/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOFRAMECHOOSER_H
#define UOFRAMECHOOSER_H

#include <QWidget>
#define FRAMECHOOSER_LINEITEM_WIDHT 27
#define FRAMECHOOSER_LINEITEM_COUNT 6
#include "uoReport.h"

class QLine;
namespace uoReport {

struct uoFC_LineItem
{
	Qt::PenStyle m_lineType;
	int m_sizeH;
	int m_offsetH;
	bool m_selelected;
	uoFC_LineItem()
	: m_lineType(Qt::NoPen),m_sizeH(FRAMECHOOSER_LINEITEM_WIDHT),m_offsetH(0),m_selelected(false)
	{}
};

class uoFrameChooser : public QWidget
{
    Q_OBJECT

	public:
		uoFrameChooser(QWidget *parent = 0);
		virtual ~uoFrameChooser();
		Qt::PenStyle penStyle();
		void setPenStyle(Qt::PenStyle style);

		uoCellBorderType borderType();
		void setBorderType(uoCellBorderType style);

		void clearType() {m_curentLine = -1;}

	public:
		QSize sizeHint() const;
		QSize minimumSizeHint() const;
	signals:
		void frameChanged(int newFrame);

	private:
		uoFrameChooser(const uoFrameChooser& other);
		uoFrameChooser& operator=(const uoFrameChooser& other);
	protected:
		void paintEvent(QPaintEvent *event);
		void keyPressEvent ( QKeyEvent * event );
        void mousePressEvent(QMouseEvent *event);
virtual	void changeEvent ( QEvent * event );
	private:
		void fillLines();
		uoFC_LineItem *m_liArray[FRAMECHOOSER_LINEITEM_COUNT+1];
		int m_curentLine;
		int m_leftOffset;
};
} //namespace uoReport
#endif // UOFRAMECHOOSER_H
