/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UORPAGESETUP_H
#define UORPAGESETUP_H

#include "ui_uorPageSetupDlg.h"

class QDialog;
struct uorPagePrintSetings;

namespace uoReport {
/**
	\class uorPageSetup - диалог настройки параметров печати.
*/

class uorPageSetup : public QDialog, Ui::uorPageSetup
{
    Q_OBJECT

	public:
		uorPageSetup(QWidget *parent = 0);
		virtual ~uorPageSetup();
		void setSettings(uorPagePrintSetings* pSetings);
	private:
		uorPagePrintSetings* m_pagesSetings;
		void populateCombo();
	private slots:
		void refreshPageLabel();
	public slots:
		virtual void accept();

};

} //namespace uoReport

#endif // UORPAGESETUP_H
