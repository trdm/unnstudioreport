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
static int uo_viewCounter = 0;

uoReportManagerMesager::uoReportManagerMesager(QObject *parent)
	:QObject(parent)
{
	m_lastMessage = "";
	m_proc = 0;
	m_marker = 0;
}

uoReportManagerMesager::~uoReportManagerMesager(){
}

void uoReportManagerMesager::setStatyMessage(QString str, const int &proc){
	m_lastMessage = str;
	m_proc = proc;
	emit onStatyMessage(m_lastMessage,m_proc);
}

void uoReportManagerMesager::setMessage(QString str, const int &marker ){
	m_lastMessage = str;
	m_marker = marker;
	emit onMessage(m_lastMessage,marker);

}



uoReportManager::uoReportManager(QObject *parent)
	:QObject(parent)
{
	m_printer = new QPrinter;
	m_progressDlg = new QProgressDialog;
	m_mainWidget = NULL;
	m_messeger = new uoReportManagerMesager;


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

int uoReportManager::nextViewCounter()
{
	++uo_viewCounter;
	return uo_viewCounter;
}

void uoReportManager::setStatyMessage(QString str, const int &proc){
	if (m_messeger){
		m_messeger->setStatyMessage(str, proc);
	}
}

void uoReportManager::setMessage(QString str, const int &marker){
	if (m_messeger){
		m_messeger->setMessage(str, marker);
		emit onMessage(str,marker);
	}
}



} //namespace uoReport {
