/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportViewIteract.h"
#include "uoReportLoader.h"
#include <QFileDialog>
#include <QMessageBox>



namespace uoReport {


uoInputIdDlg::uoInputIdDlg(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
}


uoReportViewIteract::uoReportViewIteract(QObject* parent)
	: QObject(parent)
{
	//ctor
	m_shortkatUse = false;
}

uoReportViewIteract::~uoReportViewIteract()
{
	//dtor
}

/// Создаем акции для uoReportCtrl.
void uoReportViewIteract::createActions()
{
	m_actCut 		= new QAction(QString::fromUtf8("Вырезать"),this);
	m_actCopy 		= new QAction(QString::fromUtf8("Копировать"),this);
	m_actPaste 		= new QAction(QString::fromUtf8("Вставить"),this);
	m_actDelete 	= new QAction(QString::fromUtf8("Удалить"),this);
	m_actAdd 		= new QAction(QString::fromUtf8("Добавить"),this);
	m_actClear 		= new QAction(QString::fromUtf8("Очистить"),this);
	m_actSize 		= new QAction(QString::fromUtf8("Размер..."),this);

	m_actSectionIn	= new QAction(QString::fromUtf8("Включить в секцию"),this);
	m_actSectionOut	= new QAction(QString::fromUtf8("Исключить из секции"),this);

	m_actGroupIn	= new QAction(QString::fromUtf8("Включить в группу"),this);
	m_actGroupOut	= new QAction(QString::fromUtf8("Исключить из группы"),this);

	m_actFoldTo		= new QAction(QString::fromUtf8("Свернуть"),this);
	m_actFoldUn		= new QAction(QString::fromUtf8("Развернуть"),this);

	m_actGroupShow	= new QAction(QString::fromUtf8("Показать группы"),this);
	m_actGroupHide	= new QAction(QString::fromUtf8("Скрыть группы"),this);

	m_actSectionShow= new QAction(QString::fromUtf8("Показать секции"),this);
	m_actSectionHide= new QAction(QString::fromUtf8("Скрыть секции"),this);

	m_actGridShow	= new QAction(QString::fromUtf8("Показать сетку"),this);
	m_actGridHide	= new QAction(QString::fromUtf8("Скрыть сетку"),this);

	m_actInvCharShow = new QAction(QString::fromUtf8("Показать не печ. симв."),this);
	m_actInvCharHide = new QAction(QString::fromUtf8("Скрыть не печ. симв."),this);

	m_actSheetPanShow= new QAction(QString::fromUtf8("Показать панель поиска"),this);;
	m_actSheetPanHide= new QAction(QString::fromUtf8("Скрыть панель поиска"),this);;


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

	m_actSave 	= new QAction(QString::fromUtf8("Сохранить"),this); m_actSave->setShortcut(QKeySequence(QKeySequence::Save));
	m_actSaveAs = new QAction(QString::fromUtf8("Сохранить как.."),this);
	m_actLoad 	= new QAction(QString::fromUtf8("Открыть"),this); m_actLoad->setShortcut(QKeySequence(QKeySequence::Open));

	m_showProp 	= new QAction(QString::fromUtf8("Свойства..."),this);

	m_actRowCol_Delete	= new QAction(QString::fromUtf8("Удалить"),this);
	m_actRowCol_Add 	= new QAction(QString::fromUtf8("Вставить"),this);
	m_actRow_AutoSize 	= new QAction(QString::fromUtf8("Авторазмер"),this);
	m_actRowCol_SetSize = new QAction(QString::fromUtf8("Установить размер"),this);

	m_actProperty	= new QAction(QString::fromUtf8("Свойства"),this);

	m_actCreateMatrix = new QAction(QString::fromUtf8("Создать матрицу"),this);
	m_actCreateMatrixMini = new QAction(QString::fromUtf8("Создать мини матрицу"),this);
	m_actOptions = new QAction(QString::fromUtf8("Опции..."),this);
	m_actLoad_TXT = new QAction(QString::fromUtf8("Загрузить текст"),this);


	connect(m_actScope25, SIGNAL(triggered()), this, SLOT(onScale25()));
	connect(m_actScope50, SIGNAL(triggered()), this, SLOT(onScale50()));
	connect(m_actScope75, SIGNAL(triggered()), this, SLOT(onScale75()));
	connect(m_actScope100, SIGNAL(triggered()), this, SLOT(onScale100()));
	connect(m_actScope125, SIGNAL(triggered()), this, SLOT(onScale125()));
	connect(m_actScope150, SIGNAL(triggered()), this, SLOT(onScale150()));
	connect(m_actScope200, SIGNAL(triggered()), this, SLOT(onScale200()));
	connect(m_actScope250, SIGNAL(triggered()), this, SLOT(onScale250()));
	connect(m_actScope300, SIGNAL(triggered()), this, SLOT(onScale300()));

	m_actUndo  	= new QAction(QString::fromUtf8("Отмена"),this);
	m_actRedo	= new QAction(QString::fromUtf8("Повторить"),this);
	m_actJoin	= new QAction(QString::fromUtf8("Объединить"),this);
	m_actUnJoin	= new QAction(QString::fromUtf8("Разьединить"),this);

	m_showPreview	= new QAction(QString::fromUtf8("Предварительный просмотр"),this);
	m_showPageSettings = new QAction(QString::fromUtf8("Страница"),this);

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

	connect(m_actSectionHide, 	SIGNAL(triggered()), rCtrl, SLOT(onSectionHide()));
	connect(m_actSectionShow, 	SIGNAL(triggered()), rCtrl, SLOT(onSectionShow()));

	connect(m_actSave, 			SIGNAL(triggered()), rCtrl, SLOT(onSave()));
	connect(m_actSaveAs, 		SIGNAL(triggered()), rCtrl, SLOT(onSaveAs()));

	connect(m_actLoad, 			SIGNAL(triggered()), rCtrl, SLOT(onLoad()));
	connect(m_actClear,			SIGNAL(triggered()), rCtrl, SLOT(onClear()));

	connect(m_actSectionIn,		SIGNAL(triggered()), rCtrl, SLOT(onSectionInclude()));
	connect(m_actSectionOut,	SIGNAL(triggered()), rCtrl, SLOT(onSectionExclude()));

	connect(m_actGroupIn,		SIGNAL(triggered()), rCtrl, SLOT(onGroupInclude()));
	connect(m_actGroupOut,		SIGNAL(triggered()), rCtrl, SLOT(onGroupExclude()));

	connect(m_actOutToDebug, 	SIGNAL(triggered()), rCtrl, SLOT(debugRects()));

	connect(m_actInvCharHide, 	SIGNAL(triggered()), rCtrl, SLOT(onInvisibleCharHide()));
	connect(m_actInvCharShow, 	SIGNAL(triggered()), rCtrl, SLOT(onInvisibleCharShow()));

	connect(m_actSheetPanHide, 	SIGNAL(triggered()), rCtrl, SLOT(onSheetPanHide()));
	connect(m_actSheetPanShow, 	SIGNAL(triggered()), rCtrl, SLOT(onSheetPanShow()));

	connect(m_actUndo, 			SIGNAL(triggered()), rCtrl, SLOT(onUndo()));
	connect(m_actRedo, 			SIGNAL(triggered()), rCtrl, SLOT(onRedo()));

	connect(m_actJoin, 			SIGNAL(triggered()), rCtrl, SLOT(onCellJoin()));
	connect(m_actUnJoin, 		SIGNAL(triggered()), rCtrl, SLOT(onCellUnJoin()));

	connect(m_actRowCol_Delete, 	SIGNAL(triggered()), rCtrl, SLOT(onRowColDelete()));
	connect(m_actRowCol_Add, 		SIGNAL(triggered()), rCtrl, SLOT(onRowColAdd()));
	connect(m_actRow_AutoSize, 		SIGNAL(triggered()), rCtrl, SLOT(onRowAutoSize()));
	connect(m_actRowCol_SetSize, 	SIGNAL(triggered()), rCtrl, SLOT(onRowColSetSize()));

	connect(m_showPreview, 		SIGNAL(triggered()), rCtrl, SLOT(onShowPreview()));
	connect(m_showPageSettings, 		SIGNAL(triggered()), rCtrl, SLOT(onShowPagesSetings()));

	connect(m_showProp, 		SIGNAL(triggered()), rCtrl, SLOT(propertyEditorShowActivate()));

	connect(m_actCreateMatrix, 	SIGNAL(triggered()), rCtrl, SLOT(onCreateMatrix()));
	connect(m_actCreateMatrixMini, 	SIGNAL(triggered()), rCtrl, SLOT(onCreateMatrixMini()));
	connect(m_actOptions, 		SIGNAL(triggered()), rCtrl, SLOT(onOptionsShow()));
	connect(m_actLoad_TXT, 		SIGNAL(triggered()), rCtrl, SLOT(onLoadTXT()));

	connect(m_actCut, 		SIGNAL(triggered()), rCtrl, SLOT(onCut()));
	connect(m_actPaste, 	SIGNAL(triggered()), rCtrl, SLOT(onPaste()));
	connect(m_actCopy, 	SIGNAL(triggered()), rCtrl, SLOT(onCopy()));
	if (m_shortkatUse) {
		QShortcut* shcut = 0;
		shcut = new QShortcut(QKeySequence(QKeySequence::Open),rCtrl);
		connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onLoad()));
		m_actLoad->setShortcut(QKeySequence(QKeySequence::Open));

		// Дубляж клавишь сделан намеренно, ибо на винде у меня после QKeySequence::Copy не работает Ctrl+Insert
		shcut = new QShortcut(QKeySequence(QKeySequence::Copy),rCtrl);	connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onCopy()));
		m_actCopy->setShortcut(QKeySequence(QKeySequence::Copy));

		shcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Insert),rCtrl);	connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onCopy()));
		shcut = new QShortcut(QKeySequence(QKeySequence::Paste),rCtrl);	connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onPaste()));
		shcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Insert),rCtrl);	connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onPaste()));
		m_actPaste->setShortcut(QKeySequence(QKeySequence::Paste));

		shcut = new QShortcut(QKeySequence(QKeySequence::Undo),rCtrl);	connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onUndo())); m_actUndo->setShortcut(QKeySequence(QKeySequence::Undo));
		shcut = new QShortcut(QKeySequence(QKeySequence::Redo),rCtrl);	connect(shcut, SIGNAL(activated()), rCtrl, SLOT(onRedo()));	m_actRedo->setShortcut(QKeySequence(QKeySequence::Redo));
	}

}


void uoReportViewIteract::refreshActions(uoReportCtrl* rCtrl)
{
	if (!rCtrl){
		m_actCut->setEnabled(false);
		m_actCopy->setEnabled(false);
		m_actPaste->setEnabled(false);
		m_actDelete->setEnabled(false);
		m_actAdd->setEnabled(false);
		m_actClear->setEnabled(false);
		m_actSize->setEnabled(false);

		m_actSectionIn->setEnabled(false);
		m_actSectionOut->setEnabled(false);

		m_actGroupIn->setEnabled(false);
		m_actGroupOut->setEnabled(false);

		m_actFoldTo->setEnabled(false);
		m_actFoldUn->setEnabled(false);

		m_actGroupShow->setEnabled(false);
		m_actGroupHide->setEnabled(false);

		m_actSectionShow->setEnabled(false);
		m_actSectionHide->setEnabled(false);

		m_actGridShow->setEnabled(false);
		m_actGridHide->setEnabled(false);

		m_actInvCharShow->setEnabled(false);
		m_actInvCharHide->setEnabled(false);

		m_actFrameShow->setEnabled(false);
		m_actFrameHide->setEnabled(false);

		m_actRulerShow->setEnabled(false);
		m_actRulerHide->setEnabled(false);

		m_actOutToDebug->setEnabled(false);

		m_actScope25->setEnabled(false);
		m_actScope50->setEnabled(false);
		m_actScope75->setEnabled(false);
		m_actScope100->setEnabled(false);
		m_actScope125->setEnabled(false);
		m_actScope150->setEnabled(false);
		m_actScope200->setEnabled(false);
		m_actScope250->setEnabled(false);
		m_actScope300->setEnabled(false);

		m_actSave->setEnabled(false);
		m_actSaveAs->setEnabled(false);
		m_actLoad->setEnabled(false);

		m_showProp->setEnabled(false);

		m_actRowCol_Delete->setEnabled(false);
		m_actRowCol_Add->setEnabled(false);
		m_actRow_AutoSize->setEnabled(false);
		m_actRowCol_SetSize->setEnabled(false);

		m_actProperty->setEnabled(false);
		return;
	}

	bool enabl = rCtrl->m_selections->isTrueForSections();
	uorSelectionType selMode = rCtrl->m_selections->selectionType();
	m_actRowCol_Delete->setEnabled(enabl);
	m_actRowCol_Add->setEnabled(enabl);
	m_actRow_AutoSize->setEnabled(false);
	if (selMode == uoRst_Row || selMode == uoRst_Rows){
		m_actRow_AutoSize->setEnabled(enabl);
	}
	m_actRowCol_SetSize->setEnabled(false);
	if (
	selMode == uoRst_Row ||
	selMode == uoRst_Rows ||
	selMode == uoRst_Column ||
	selMode == uoRst_Columns
	){
		m_actRowCol_SetSize->setEnabled(true);
	}


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

bool uoReportViewIteract::chooseLoadTxtFilePath(QString& filePath, QWidget* wi)
{
    Q_UNUSED(wi);
	QString filePathThis = filePath;
	if (filePathThis.isEmpty()){
		filePathThis = "report.txt";
	}

	QString fileName = QFileDialog::getOpenFileName(0,
						 tr("Load txt.."),
						 filePathThis,
						 tr("Txt Files (*.txt)"));
	if (!fileName.isEmpty()){
		filePath = fileName;
		return true;
	}
	return false;

}

/// Выбрать имя файла и формат для считывания
bool uoReportViewIteract::chooseLoadFilePathAndFormat(QString& filePath, uoRptStoreFormat& frmt, QWidget* wi)
{
	QString filePathThis = filePath;
	if (filePathThis.isEmpty()){
		filePathThis = "report.xml";
	}
	QString filter;
	QMap<uoRptStoreFormat,QString> map = uoReportLoader::getAviableLoadFormat(filter);

	QString fileName = QFileDialog::getOpenFileName(0,
						 tr("Load report.."),
						 filePathThis,
						 filter //tr("XML Files (*.xml)")
						 );
	if (!fileName.isEmpty()){
		frmt = uoReportLoader::getFormatByName(fileName);
		if (uoRsf_Unknown == frmt){
			QMessageBox::information(wi, tr("Attention"), tr("Not correct file name"));
			return false;
		}
		// Пока остальные оставим.
		filePath = fileName;
		return true;
	}
	return false;

}
/// Выбрать имя файла и формат
bool uoReportViewIteract::chooseSaveFilePathAndFormat(QString& filePath, uoRptStoreFormat& frmt, QWidget* wi )
{

	if (filePath.isEmpty()){
		filePath = "report.xml";
	}

	QFileDialog::Options options;

	QString filter;
	QMap<uoRptStoreFormat,QString> map = uoReportLoader::getAviableStoreFormat(filter);

	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(wi,
						 tr("Save report.."),
						 filePath,
						 filter,
						 &selectedFilter,
						 options);

	if (!fileName.isEmpty()){
		frmt = uoReportLoader::getFormatByName(fileName);
		if (uoRsf_Unknown == frmt){
			QMessageBox::information(wi, tr("Attention"), tr("Not correct file name"));
			return false;
		}
		// Пока остальные оставим.
		filePath = fileName;
		return true;
	}

	return false;
}

/// Запрос на ввод/изменение имени секции...
bool uoReportViewIteract::inputSectionName(QString& name, QWidget* wi)
{
	uoInputIdDlg* dlg = new uoInputIdDlg(wi);
	if (!dlg)
		return false;
	bool retVal = false;
	dlg->m_name->setText(name);
	int dret = dlg->exec();
	if (dret == QDialog::Accepted){
		retVal = true;
		name = dlg->m_name->text();
		if (name.isEmpty())
			retVal = false;
	}
	delete  dlg;
	return retVal;
}


void uoReportViewIteract::onScale25(){	emit onScaleChange(0.25);}
void uoReportViewIteract::onScale50(){	emit onScaleChange(0.5);}
void uoReportViewIteract::onScale75(){	emit onScaleChange(0.75);}
void uoReportViewIteract::onScale100(){	emit onScaleChange(1.0);}
void uoReportViewIteract::onScale125(){	emit onScaleChange(1.25);}
void uoReportViewIteract::onScale150(){	emit onScaleChange(1.5);}
void uoReportViewIteract::onScale200(){	emit onScaleChange(2);}
void uoReportViewIteract::onScale250(){	emit onScaleChange(2.5);}
void uoReportViewIteract::onScale300(){	emit onScaleChange(3.0);}


} //namespace uoReport
