/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTMANAGER_H
#define UOREPORTMANAGER_H

class QPrinter;
class QPrintDialog;

namespace uoReport {

class uoReportManager
{
	protected:
		static uoReportManager* m_self;
		uoReportManager();
		~uoReportManager();
	public:
		static uoReportManager* instance()	{
			if (!m_self){
				m_self = new uoReportManager;
			}
			return m_self;
		}
		QPrinter *printer();
	protected:
		QPrinter* 		m_printer;

};

} //namespace uoReport {

#endif // UOREPORTMANAGER_H
