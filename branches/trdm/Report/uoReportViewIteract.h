/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTVIEWITERACT_H
#define UOREPORTVIEWITERACT_H

#include <QAction>
#include "uoReport.h"
#include "uoReportCtrl.h"

namespace uoReport {

/**
	\class uoReportViewIteract
	\brief Помошник интерактивного взаимодеййствия с пользователем...
	выносим сюда всякие нудные неконцептуальные массивные вещи, к примеру
	инициализации множества итемов.

	В итоге, это будет отдельный разделяемый класс, наполняющий интерфейс.

*/

class uoReportViewIteract : public QObject
{
	Q_OBJECT
	public:
		uoReportViewIteract(QObject* parent = 0);
		virtual ~uoReportViewIteract();

		void createActions();
		void connectActions(uoReportCtrl* rCtrl);

		QAction* m_actCut;
		QAction* m_actRemember;
		QAction* m_actDelete;
		QAction* m_actAdd;
		QAction* m_actClear;
		QAction* m_actSize;
		QAction* m_actSectionIn;
		QAction* m_actSectionOut;
		QAction* m_actFoldTo;
		QAction* m_actFoldUn;
		QAction* m_actProperty;

		QAction* m_actGroupShow;
		QAction* m_actGroupHide;

		QAction* m_actSectionShow;
		QAction* m_actSectionHide;

		QAction* m_actGridShow;
		QAction* m_actGridHide;

		QAction* m_actRulerShow;
		QAction* m_actRulerHide;

		QAction* m_actFrameShow;
		QAction* m_actFrameHide;

		QAction* m_actScope25;
		QAction* m_actScope50;
		QAction* m_actScope75;
		QAction* m_actScope100;
		QAction* m_actScope125;
		QAction* m_actScope150;
		QAction* m_actScope200;
		QAction* m_actScope250;
		QAction* m_actScope300;

		QAction* m_actOutToDebug;

		QAction* m_actSave;
		QAction* m_actSaveAs;
		QAction* m_actLoad;

		void setCheckedState(qreal scaleFactor);

	signals:
		void onScaleChange(const qreal scaleFact);

	private slots:
		void onScale25();
		void onScale50();
		void onScale75();
		void onScale100();
		void onScale125();
		void onScale150();
		void onScale200();
		void onScale250();
		void onScale300();

};

} //namespace uoReport

#endif // UOREPORTVIEWITERACT_H
