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
class QString;
#include <QObject>

namespace uoReport {

class uoReportManagerMesager : public QObject
{
	Q_OBJECT
public:
	uoReportManagerMesager(QObject* parent = 0);
	virtual ~uoReportManagerMesager();
	void setStatyMessage(QString str, const int& proc = -1);
	void setMessage(QString str, const int& marker = -1);

signals:
	void onStatyMessage(const QString& message, const int& proc = -1); /// сообшение для строки состояния
	void onMessage(const QString& message, const int& marker); /// сообшение для окна сообщения состояния
protected:
	QString	m_lastMessage;
	int		m_proc;
	int		m_marker;

};

///\todo отладить в винде, где не установлены принтера, что-бы ругани небыло...
class uoReportManager : public QObject
{
	Q_OBJECT
	protected:
		static uoReportManager* m_self;
		uoReportManager(QObject* parent= 0);
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
		static int	nextViewCounter();
		void setStatyMessage(QString  str, const int& proc = -1);
		void setMessage(QString str, const int& marker = -1);
	signals:
		void onStatyMessage(const QString& message, const int& proc = -1); /// сообшение для строки состояния
		void onMessage(const QString& message, const int& marker); /// сообшение для окна сообщения состояния
	protected:
		QPrinter* 			m_printer;
		QProgressDialog* 	m_progressDlg;
		QWidget*			m_mainWidget;
		QString				m_lastMessadge;
		uoReportManagerMesager* m_messeger;

};

} //namespace uoReport {

#endif // UOREPORTMANAGER_H
