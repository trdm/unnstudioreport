/**************************************
*
*	© trdm, as Troshin D. V. 2010
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoSheetPanel.h"
#include <QResizeEvent>

namespace uoReport {

uoSheetPanel::uoSheetPanel(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	hide();

}

uoSheetPanel::~uoSheetPanel()
{
	//dtor
}

void uoSheetPanel::setAdress(const QString& adressString )
{
	m_AdressCellEdit->setText(adressString);
}

void uoSheetPanel::setText(const QString& textString )
{
	m_cellText->setText(textString); // m_FindEdit
}

void uoSheetPanel::setFindText(const QString& textFind )
{
	m_FindEdit->setText(textFind);
}


void uoSheetPanel::resizeEvent ( QResizeEvent * event )
{
	QWidget::resizeEvent(event);
}


} //namespace uoReport {
