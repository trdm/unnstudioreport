/**************************************
*
*	© trdm, as Troshin D. V. 2010
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOSHEETPANEL_H
#define UOSHEETPANEL_H

#include <QWidget>
#include "ui_uorSheetPanel.h"

namespace uoReport {

class uoSheetPanel : public QWidget, Ui::uorSheetPanel
{
	Q_OBJECT
	public:

		uoSheetPanel(QWidget *parent = 0);

		virtual ~uoSheetPanel();
		void setAdress(const QString& adressString );
		void setText(const QString& textString );
		void setFindText(const QString& textFind );

	protected:
		void resizeEvent ( QResizeEvent * event );
	private:
};

} //namespace uoReport {

#endif // UOSHEETPANEL_H
