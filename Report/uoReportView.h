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

///\class uoReportView - контрол для размещения всех чилдов репорта. Просто прослойка.
///\brief uoReportView - контрол для размещения всех чилдов репорта. Просто прослойка.
class uoReportView : public QFrame
{
	Q_OBJECT
	public:
		uoReportView(QWidget *parent = 0);
		virtual ~uoReportView();
	public:
		uoReportCtrl* getControl() {return m_ctrl;}
        void resize(int w, int h);
    protected:
		void resizeEvent ( QResizeEvent * event );
	private:
		uoReportCtrl* m_ctrl;
		//uoSheetPanel* m_sPanel;

};

} //namespace uoReport
#endif // UOREPORTVIEW_H
