/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uorOptionsDlg.h"

namespace uoReport {

uorOptionsDlg::uorOptionsDlg(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	hide();
	m_DebugInfoText->setAcceptRichText(false);

}

uorOptionsDlg::~uorOptionsDlg()
{
	//dtor
}

bool uorOptionsDlg::saveWithSelection()
{
	Qt::CheckState state = m_saveWithSelection->checkState();
	return (state == Qt::Checked) ? true : false;
}

bool uorOptionsDlg::directDraw(){
	Qt::CheckState state = m_directDrawFlag->checkState();
	return (state == Qt::Checked) ? true : false;
}

void uorOptionsDlg::setSaveWithSelection(bool saveWS)
{
	if (saveWS) {
		m_saveWithSelection->setCheckState(Qt::Checked);
	} else {
		m_saveWithSelection->setCheckState(Qt::Unchecked);
	}
}

void uorOptionsDlg::setDirectDrawFlag(bool dd)
{
	m_directDrawFlag->setCheckState( dd ? Qt::Checked : Qt::Unchecked);
}

void uorOptionsDlg::setDebugString(const QString& toDebug)
{
	m_DebugInfoText->setPlainText(toDebug);
}

void uorOptionsDlg::setSaveWithUndoStack(bool saveWS)
{
	if (saveWS) {
		m_saveWithUndoStack->setCheckState(Qt::Checked);
	} else {
		m_saveWithUndoStack->setCheckState(Qt::Unchecked);
	}

}
bool uorOptionsDlg::isSaveWithUndoStack()
{
	Qt::CheckState state = m_saveWithUndoStack->checkState();
	return (state == Qt::Checked) ? true : false;
}



} //namespace uoReport {
