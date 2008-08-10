/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTVIEW_H
#define UOREPORTVIEW_H

#include <QFrame>
#include "uoReport.h"
#include "uoReportCtrl.h"
namespace uoReport {

class uoReportView : public QFrame
{
	Q_OBJECT
	public:
		uoReportView(QWidget *parent = 0);
		virtual ~uoReportView();
	public:
		void resize(int w, int h);
		uoReportCtrl* getControl() {return _ctrl;};
	protected:
		void resizeEvent ( QResizeEvent * event );
	private:
		uoReportCtrl* _ctrl;

};

} //namespace uoReport
#endif // UOREPORTVIEW_H
