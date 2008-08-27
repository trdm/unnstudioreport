/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportViewIteract.h"
#include <QFileDialog>
#include <QMessageBox>


namespace uoReport {

uoReportViewIteract::uoReportViewIteract(QObject* parent)
	: QObject(parent)
{
	//ctor
}

uoReportViewIteract::~uoReportViewIteract()
{
	//dtor
}

/// Создаем акции для uoReportCtrl.
void uoReportViewIteract::createActions()
{
	m_actCut 		= new QAction(QString::fromUtf8("Вырезать"),this);
	m_actRemember 	= new QAction(QString::fromUtf8("Запомнить"),this);
	m_actDelete 	= new QAction(QString::fromUtf8("Удалить"),this);
	m_actAdd 		= new QAction(QString::fromUtf8("Добавить"),this);
	m_actClear 		= new QAction(QString::fromUtf8("Очистить"),this);
	m_actSize 		= new QAction(QString::fromUtf8("Размер..."),this);
	m_actSectionIn	= new QAction(QString::fromUtf8("Включить в секцию"),this);
	m_actSectionOut	= new QAction(QString::fromUtf8("Изключить в секцию"),this);
	m_actFoldTo		= new QAction(QString::fromUtf8("Свернуть"),this);
	m_actFoldUn		= new QAction(QString::fromUtf8("Развернуть"),this);

	m_actGroupShow	= new QAction(QString::fromUtf8("Показать группы"),this);
	m_actGroupHide	= new QAction(QString::fromUtf8("Скрыть группы"),this);

	m_actSectionShow= new QAction(QString::fromUtf8("Показать секции"),this);
	m_actSectionHide= new QAction(QString::fromUtf8("Скрыть секции"),this);

	m_actGridShow	= new QAction(QString::fromUtf8("Показать сетку"),this);
	m_actGridHide	= new QAction(QString::fromUtf8("Скрыть сетку"),this);

	m_actFrameShow 	= new QAction(QString::fromUtf8("Показать рамку"),this);
	m_actFrameHide	= new QAction(QString::fromUtf8("Скрыть рамку"),this);

	m_actRulerShow 	= new QAction(QString::fromUtf8("Показать линейку"),this);
	m_actRulerHide 	= new QAction(QString::fromUtf8("Скрыть линейку"),this);

	m_actOutToDebug = new QAction(QString::fromUtf8("Отладочная информация>>>"),this);

	m_actScope25 = new QAction(QString::fromUtf8("25%"),this); m_actScope25->setCheckable(true);
	m_actScope50 = new QAction(QString::fromUtf8("50%"),this); m_actScope50->setCheckable(true);
	m_actScope75 = new QAction(QString::fromUtf8("75%"),this); m_actScope75->setCheckable(true);
	m_actScope100 = new QAction(QString::fromUtf8("100%"),this); m_actScope100->setCheckable(true);
	m_actScope125 = new QAction(QString::fromUtf8("125%"),this); m_actScope125->setCheckable(true);
	m_actScope150 = new QAction(QString::fromUtf8("150%"),this); m_actScope150->setCheckable(true);
	m_actScope200 = new QAction(QString::fromUtf8("200%"),this); m_actScope200->setCheckable(true);
	m_actScope250 = new QAction(QString::fromUtf8("250%"),this); m_actScope250->setCheckable(true);
	m_actScope300 = new QAction(QString::fromUtf8("300%"),this); m_actScope300->setCheckable(true);

	m_actSave 	= new QAction(QString::fromUtf8("Сохранить"),this);
	m_actSaveAs = new QAction(QString::fromUtf8("Сохранить как.."),this);
	m_actLoad 	= new QAction(QString::fromUtf8("Открыть"),this);


	m_actProperty	= new QAction(QString::fromUtf8("Свойства"),this);

	connect(m_actScope25, SIGNAL(triggered()), this, SLOT(onScale25()));
	connect(m_actScope50, SIGNAL(triggered()), this, SLOT(onScale50()));
	connect(m_actScope75, SIGNAL(triggered()), this, SLOT(onScale75()));
	connect(m_actScope100, SIGNAL(triggered()), this, SLOT(onScale100()));
	connect(m_actScope125, SIGNAL(triggered()), this, SLOT(onScale125()));
	connect(m_actScope150, SIGNAL(triggered()), this, SLOT(onScale150()));
	connect(m_actScope200, SIGNAL(triggered()), this, SLOT(onScale200()));
	connect(m_actScope250, SIGNAL(triggered()), this, SLOT(onScale250()));
	connect(m_actScope300, SIGNAL(triggered()), this, SLOT(onScale300()));
}

/// коннектим акции итеракта к uoReportCtrl.
void uoReportViewIteract::connectActions(uoReportCtrl* rCtrl)
{
	connect(m_actGridHide, SIGNAL(triggered()), rCtrl, SLOT(onGridHide()));
	connect(m_actGridShow, SIGNAL(triggered()), rCtrl, SLOT(onGridShow()));

	connect(m_actGroupHide, SIGNAL(triggered()), rCtrl, SLOT(onGroupHide()));
	connect(m_actGroupShow, SIGNAL(triggered()), rCtrl, SLOT(onGroupShow()));

	connect(m_actRulerHide, SIGNAL(triggered()), rCtrl, SLOT(onRulerHide()));
	connect(m_actRulerShow, SIGNAL(triggered()), rCtrl, SLOT(onRulerShow()));

	connect(m_actSectionHide, SIGNAL(triggered()), rCtrl, SLOT(onSectionHide()));
	connect(m_actSectionShow, SIGNAL(triggered()), rCtrl, SLOT(onSectionShow()));

	connect(m_actSave, 	SIGNAL(triggered()), rCtrl, SLOT(onSave()));
	connect(m_actSaveAs, 	SIGNAL(triggered()), rCtrl, SLOT(onSaveAs()));

	connect(m_actOutToDebug, 	SIGNAL(triggered()), rCtrl, SLOT(debugRects()));


}

void uoReportViewIteract::setCheckedState(qreal scaleFactor){
	m_actScope25->setChecked(false);
	m_actScope50->setChecked(false);
	m_actScope75->setChecked(false);
	m_actScope100->setChecked(false);
	m_actScope125->setChecked(false);
	m_actScope150->setChecked(false);
	m_actScope200->setChecked(false);
	m_actScope250->setChecked(false);
	m_actScope300->setChecked(false);

	if 		(scaleFactor == 0.25) 	{		m_actScope25->setChecked(true);	}
	else if (scaleFactor == 0.5) 	{		m_actScope50->setChecked(true);	}
	else if (scaleFactor == 0.75) 	{		m_actScope75->setChecked(true);	}
	else if (scaleFactor == 1) 		{		m_actScope100->setChecked(true);	}
	else if (scaleFactor == 1.25) 	{		m_actScope125->setChecked(true);	}
	else if (scaleFactor == 1.5) 	{		m_actScope150->setChecked(true);	}
	else if (scaleFactor == 2.0) 	{		m_actScope200->setChecked(true);	}
	else if (scaleFactor == 2.5) 	{		m_actScope250->setChecked(true);	}
	else if (scaleFactor == 3.0) 	{		m_actScope300->setChecked(true);	}
}

/// Выбрать имя файла и формат
bool uoReportViewIteract::chooseSaveFilePathAndFormat(QString& filePath, uoRptStoreFormat& frmt, QWidget* wi )
{
	bool retVal = false;

	if (filePath.isEmpty()){
		filePath = "report.xml";
	}

	QFileDialog::Options options;

	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(wi,
						 tr("Save report.."),
						 filePath,
						 tr("XML Files (*.xml);*.xml"),
						 &selectedFilter,
						 options);
	if (!fileName.isEmpty()){
		frmt = uoRsf_Unknown;
		if (fileName.endsWith(QString(".xml"), Qt::CaseInsensitive)){
			frmt = uoRsf_XML;
		} else {
			QMessageBox::information(wi, tr("Attention"), tr("Not correct file name"));
			return false;
		}
		// Пока остальные оставим.
		filePath = fileName;
		return true;
	}
	return false;
}


void uoReportViewIteract::onScale25(){	emit onScaleChange(0.25);}
void uoReportViewIteract::onScale50(){	emit onScaleChange(0.5);}
void uoReportViewIteract::onScale75(){	emit onScaleChange(0.75);}
void uoReportViewIteract::onScale100(){	emit onScaleChange(1);}
void uoReportViewIteract::onScale125(){	emit onScaleChange(1.25);}
void uoReportViewIteract::onScale150(){	emit onScaleChange(1.5);}
void uoReportViewIteract::onScale200(){	emit onScaleChange(2);}
void uoReportViewIteract::onScale250(){	emit onScaleChange(2.5);}
void uoReportViewIteract::onScale300(){	emit onScaleChange(3.0);}


} //namespace uoReport
