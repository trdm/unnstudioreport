/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportView.h"

namespace uoReport {

uoReportView::uoReportView(QWidget *parent)
	: QFrame(parent)
{
	_ctrl = new uoReportCtrl(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

uoReportView::~uoReportView()
{
	//dtor
}

void uoReportView::resizeEvent( QResizeEvent * event ){
	QFrame::resizeEvent(event);
	_ctrl->resize(event->size());
	_ctrl->move(0,0);
}

void uoReportView::resize(int w, int h){
	QFrame::resize(w, h);
	_ctrl->resize(w, h);
}

} //namespace uoReport
