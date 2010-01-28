/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportManager.h"
#include <QPrinter>
#include <QProgressDialog>
#include <QApplication>
#include <QWidget>

namespace uoReport {

uoReportManager* uoReportManager::m_self = 0;

uoReportManager::uoReportManager()
{
	m_printer = new QPrinter;
	m_progressDlg = new QProgressDialog;
	m_mainWidget = NULL;


}

uoReportManager::~uoReportManager()
{
	//dtor
}

QPrinter* uoReportManager::printer()
{
	return m_printer;
}

QProgressDialog* uoReportManager::progressDlg()
{
	if (m_mainWidget && m_progressDlg)
		m_progressDlg->setParent(m_mainWidget);
	return m_progressDlg;
}

void uoReportManager::setMainWidget(QWidget* wi)
{
	m_mainWidget = wi;
}


} //namespace uoReport {
