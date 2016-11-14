/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOROPTIONSDLG_H
#define UOROPTIONSDLG_H

#include <QDialog>
#include "ui_uorOptions.h"

namespace uoReport {

class uorOptionsDlg : public QDialog, Ui::uorOptionsDlgUi
{
	Q_OBJECT
	public:
		uorOptionsDlg(QWidget *parent = 0);
		virtual ~uorOptionsDlg();
		bool saveWithSelection();
		bool directDraw();
		void setSaveWithSelection(bool saveWS);
		void setDebugString(const QString& toDebug);
		void setSaveWithUndoStack(bool saveWS);
		void setDirectDrawFlag(bool dd);
		void setUsePixCacher(bool dd);
		bool usePixCacher();
		bool isSaveWithUndoStack();
	protected:
	private:
};

} //namespace uoReport {

#endif // UOROPTIONSDLG_H
