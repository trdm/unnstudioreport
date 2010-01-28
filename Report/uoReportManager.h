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
class QProgressDialog;
class QWidget;

namespace uoReport {

///\todo отладить в винде, где не установлены принтера, что-бы ругани небыло...

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
		QProgressDialog* progressDlg();
		void setMainWidget(QWidget* wi);
	protected:
		QPrinter* 			m_printer;
		QProgressDialog* 	m_progressDlg;
		QWidget*			m_mainWidget;

};

} //namespace uoReport {

#endif // UOREPORTMANAGER_H
