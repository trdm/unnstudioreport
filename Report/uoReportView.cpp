/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportView.h"
#include <QGridLayout>

namespace uoReport {

uoReportView::uoReportView(QWidget *parent)
	: QFrame(parent)
{

	m_ctrl = new uoReportCtrl(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

}

uoReportView::~uoReportView()
{
	//dtor
}

void uoReportView::resizeEvent( QResizeEvent * event ){
	QFrame::resizeEvent(event);
}

void uoReportView::resize(int w, int h){
	QFrame::resize(w, h);
}

} //namespace uoReport
