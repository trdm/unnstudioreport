/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportManager.h"
#include <QPrinter>

namespace uoReport {

uoReportManager* uoReportManager::m_self = 0;

uoReportManager::uoReportManager()
{
	m_printer = new QPrinter;


}

uoReportManager::~uoReportManager()
{
	//dtor
}

QPrinter* uoReportManager::printer()
{
	return m_printer;
}


} //namespace uoReport {
