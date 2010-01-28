/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTDOC_H
#define UOREPORTDOC_H

#include <QObject>
#include <QLinkedList>
#include <QList>
#include <QSet>
#include <QPrinter>
#include <QPrintDialog>
#include <QFontMetricsF>
#include "uoReport.h"
#include "uoSpanTree.h"
#include "uoCacheItemizer.h"
#include "uorReportAreas.h"


class QObject;
class QFontMetricsF;
class uoRowsDoc;
class uoSpanTree;
class QString;
class uoHeaderScale;
struct uoTextTrPointCash;
struct uoLineSpan;
struct uoCellJoin;
class uoReportUndo;
class uoReportManager;
class uorMimeData;
class uoReportSelection;
class uoCellMatrix;



class uoCacheItemizer;
struct uorTextDecorBase;


namespace uoReport {



/**
	\class uoReportDoc - содержит данные отчета.
	\brief содержит и обслуживает данные таблицы отчета: строки, текст, картинки и т.п.
*/
class uoReportDoc : public QObject
{
    Q_OBJECT

    friend class uoReportCtrl;
	public:
		uoReportDoc();
		virtual ~uoReportDoc();
	public:
		void clear();
		void clearFonts();
		bool addGroup(int start, int end, uoRptHeaderType ht, bool folded = false);
		bool addSection(int start, int end, uoRptHeaderType ht, QString name);
		uoSpanTree* getSectionManager(uoRptHeaderType ht);
		uoSpanTree* getGroupManager(uoRptHeaderType ht);

		bool possiblyAddGroup(int start, int end, uoRptHeaderType ht);
		bool possiblyAddSection(int start, int end, uoRptHeaderType ht);

		void doDeleteColumns(int itemStart, int count = 1);
		void doDeleteRows(int itemStart, int count = 1);

		void doAddColumns(int itemStart, int count = 1);
		void doAddRows(int itemStart, int count = 1);
		const spanList* getGroupList(uoRptHeaderType rht, int start, int end);
		const spanList* getSectionList(uoRptHeaderType rht, int start, int end);

		int getGroupLevel(uoRptHeaderType ht);
		int getSectionLevel(uoRptHeaderType ht);

		void doGroupFold(int idGrop, uoRptHeaderType rht, bool fold);
		bool enableCollectChanges(const bool enable);
		bool isCollectChanges() const;

		bool save(uoReportSelection* sel = 0);
		bool load();
		bool loadFromFile(QString path, uoRptStoreFormat stFormat);
		bool saveToFile(QString path, uoRptStoreFormat stFormat, uoReportSelection* sel = 0);

		bool saveToByteArray(QByteArray& byteArr, uoReportSelection* sel);
		bool loadFromByteArray(QByteArray& byteArr, uoReportSelection* sel =0 );

		bool saveOptionsIsValid();
		bool flush(uoReportLoader* loader, uoReportSelection* selection = 0);
		uorMimeData* createMimeData(uoReportSelection* selection = 0);


		uoRptStoreFormat 	getStoreFormat(); ///< Возвращает установленный формат сохранения
		QString 			getStorePathFile();///< Возвращает установленное имя файла.
		void 				setStoreOptions(QString  filePath, uoRptStoreFormat stFormat); ///< установим опции сохранения

	public:
		void 	test();
		uorNumber 	getScalesSize(const uoRptHeaderType& hType, const int& nomStart, const int& nomEnd, const bool& ignoreHiden = false, const bool& isDef = false) const;

		uorNumber 	getScaleSize(const uoRptHeaderType& hType, const int& nom, bool isDef = false);
		void 	setScaleSize(const uoRptHeaderType& hType, const int& nom, uorNumber size, bool isDef = false);
		void 	setScaleFixedProp(const uoRptHeaderType& hType, const int& nom, bool isFixed = true);
		bool 	getScaleFixedProp(const uoRptHeaderType& hType, const int& nom);

		void 	setScalesSize(const uoRptHeaderType& hType, const QList<int>& list, const uorNumber& size, const bool& isDef = false);
		void 	setRowAutoSize(const QList<int>& list);

		void 	setScalesHide(uoRptHeaderType hType, int nmStart, int cnt = 1,  bool hide = true);

		bool 	getScaleHide(const uoRptHeaderType& hType, const int& nom);
		bool 	getRowHide(const int& rowNom);
		bool 	getColHide(const int& colNom);

		bool 	joinCells(QRect& joinRect, bool join = true);
		bool 	joinCells(int colSt, int rowSt, int colEnd, int rowEnd, bool join = true);
		uoCell* getFirstUnionCell(uoCell* curCell, int rowNo);
		void 	clearCell(uoCell* cell);

	private:
		void onAccessRowCol(int nmRow = 0, int nmCol = 0); ///< при доступе к строке или столбцу documenta...
		void onAccessRowOrCol(int nom, uoRptHeaderType rht, bool write = false);
		void doRowCountChange(int count, int pos = 0 );
		void doColCountChange(int count, int pos = 0 );
		void beforeAddRowOrCol(int count, uoRptHeaderType rht, int noLn = 0);

	public:
		///\todo Сделать получение и калькуляцию размеров документа.
		uorNumber  	getDefScaleSize(uoRptHeaderType rht);
		uorNumber  	getVSize(bool visible = false);
		uorNumber  	getHSize(bool visible = false);

		int 	getRowCount();
		int 	getColCount();

	signals:
//		void onSizeChange(uorNumber sizeV, uorNumber sizeH, int row, int col);
		void onSizeChange(uorNumber sizeV, uorNumber sizeH);
		void onSizeVisibleChangeV(uorNumber newSize, int newCount, uorNumber oldSize, int oldCnt, int pos = 0);
		void onSizeVisibleChangeH(uorNumber newSize, int newCount, uorNumber oldSize, int oldCnt, int pos = 0);

	public slots:
		void onUndo();
		void onRedo();
		void onDataChange();
		void onPrint();

		// тут получаем комманды от меню
		void 	onCopy(uoReportSelection* sel = 0, bool withDelete = false);
		void 	onPaste(uoReportSelection* sel = 0);
		void 	onPasteText(uoReportSelection* sel, const QMimeData* mime);
		void 	onPasteDocum(uoReportSelection* sel, const QMimeData* mime);
		void 	onCut(uoReportSelection* sel = 0);


	protected:
		void 	initTextDecorDoc();
		void 	copyDocum(uoReportDoc* fromDoc);


	protected:
		bool 	m_saveWithUndoStack;
	public:

inline 	bool 	isSaveUndoStack() const { return m_saveWithUndoStack; }
inline 	void 	setSaveUndoStack(const bool save){ m_saveWithUndoStack = save; }

	protected:

		uorNumber m_sizeV, m_sizeV_visible;		///< Размер документа по вертикали полный и видимый.
		uorNumber m_sizeH, m_sizeH_visible;		///< Размер документа по горизонтали полный и видимый.
		/// "Видимый" - не вьювпорт, а размеры не скрытых секций

		int m_rowCount;
		int m_colCount;

		int m_freezEvent;	/// заморозить посылку сообщений на перерисовку

		QString 		 m_docFilePath;		///< Имя файла
		uoRptStoreFormat m_storeFormat;	///< Формат хранения файла отчета.

		////////////////Данные документа {{{{{{{{{
		// Группы
		uoSpanTree* 	m_spanTreeGrCol;
		uoSpanTree* 	m_spanTreeGrRow;

		// Секции.
		uoSpanTree* 	m_spanTreeSctCol;
		uoSpanTree* 	m_spanTreeSctRow;

		uoHeaderScale* 	m_headerRow; ///< Вертикальный заголовок
		uoHeaderScale* 	m_headerCol; ///< Горизонтальный заголовок
		uoTextTrPointCash* 	m_pointBlock;

		uoRowsDoc* 	m_rows;		 				///< Значимое содержимое документа, содержимое ячеек документа.
		uoReportDocFontColl* 	m_fontColl;
		int 				 	m_defaultFontId;
		uorTextDecor* 			m_TextDecorDoc;
		uorPagePrintSetings* 	m_pagesSetings;
		long 					m_changes; ///< Счетчик изменений документа, при каждом изменении прибавляется единица.
		int 					m_ident; /// поменять на гуид или нечто подобное (есть класс QUuid)
		uoCell* 				m_cellDefault; ///< Дефолтная ячейка, содержащая значения форматирования документа по умолчанию...

		////////////////Данные документа }}}}}}}}} ниче не забыл? О_о..

		uoReportUndo* m_undoManager;
		uoCellMatrix* m_matrixCell;
		uoCacheItemizer<uoCellJoin> 		m_cellJoinCash;
		uoCacheItemizer<uorTextDecor> 		m_cellTextDecorList;
		uoCacheItemizer<uorBorderPropBase> 	m_cellBordPropList;

		uorAresList* 			m_pageList;
		uorPageColSegmentList 	m_segmentList; ///<Список структур (uorPageColSegment) для визуального разделения полотна на страницы.
		QRect 	m_paperRectPrint;	///< размер страницы в принтере
		QRect 	m_pageRectPrint;
		/**
			ряды страниц...
			Условно - "[]" - страница.
			если вертикальный размер полотна превышает ширину страницы,
			то вырисовываются ряды:
			[][][]
			[][][]
			получается 3 ряда это значение - m_pagesColumnTotal
		*/
		int 					m_pagesColumnTotal;

	private:
		void updatePageSegmentList();
		bool updatePageRectPrint();


	protected:
		// Содержимое строк документа
		QList<uoReportCtrl*> m_atachedView; 	///< Приватаченные вьювы
		QList<QObject*> 	 m_atachedObj;		///< Приватаченные объекты. Документ может использоваться без вьюва, наример для заполнения его в модуле.
		long 				 m_refCounter;
		bool 				 m_formatEnable;	///< Разрешает форматирование документа. Запретить полезно, например при считывании его с диска, что-бы не форматилось при пополнении каждой ячейке.
		uoReportSelection* 	 m_selection;		///< Восстановленное из файла выделение.
//		QList<int>			 m_rowsFormated;	///< Отформатированные строки. Помечаются во время форматирования, Нужны для того, что-бы расчитать высоту объединенных ячеек.

	public:
		void attachView(uoReportCtrl* rCtrl, bool autoConnect = true);
		void detachedView(uoReportCtrl* rCtrl);

		void atacheObject(QObject* rObj);
		void detachedObject(QObject* rObj);

		bool 	isObjectAttached();
		void 	setCellText(const int& row, const int& col, const QString& text, uorCellTextType type = uorCTT_Text);
		QString getCellText(const int& row, const int& col, uorCellTextType type = uorCTT_Text);

		///\todo - имплементировать... setCellFontProp
//		void 	setCellFontProp(const int idFont, const int posX, const QString text);
		void 	setCellTextAlignment(const int rowNo, const int colNo, uoVertAlignment va,  uoHorAlignment ha, uoCellTextBehavior tb = uoCTB_Auto);

		uoCell* getCell(const int& rowNo, const int& colNo, bool needCreate = false, bool provideProp = false);
inline	uoCell* getCellDefault() { return m_cellDefault;}
		uoRow* getRow(const int& rowNum);

		uorAresList* 	getPageList();

inline 	uoCellMatrix* 	getCellMatrix() const {return m_matrixCell;}
inline 	uoReportUndo* 	getUndoManager() const {return m_undoManager;}

		void 	setCellMatrix(uoCellMatrix* matrix);

		bool	printDoc(const bool updPrintSourse = true, QWidget* qwidg = NULL );
		bool 	setupPrinter(QPrinter &printer, QWidget* qwidg = NULL);

inline	QRect 	getPageRectPrint() const {return m_pageRectPrint;}
inline	QRect 	getPaperRectPrint() const {return m_paperRectPrint;}

		QRect 	m_needFormatArea; ///< Зона, в которой необходимо переформатировать ячейки. Оптимизация для выделений.
inline 	void 	clearFormatArea() {		m_needFormatArea = QRect(0,0,0,0);		};

inline 	uoReportSelection* 	 selection() {return m_selection; }

		void 	doFormatDoc(int nmRow = -1, int nmForCol = -1);
		void 	doFormatDocWithSelection(uoReportSelection * selection = 0);

		void 	doFormatRow(uoRow* row, int nmForCol = -1);
		void 	doFormatRowLRMaxLng(uoRow* row);
		void 	doCalculateCellUnionSizeHW(uoCell* psCell);
		void 	doFormatRow(int nmRow, int nmForCol = -1);
		uorNumber 	doFormatCellText(uoCell* cell, QFont* font, QFontMetricsF& fm, const uorNumber& collWidth );
		bool 	enableFormating(const bool format);

		bool 	isFormatEnable() {return m_formatEnable;}

		int 	getNextCellNumber(const int& rowCur, const int& colCur, const bool& ignoreHiden = true);
		QStringList splitQStringToWord(const QFontMetricsF& fm, const QString& str, const uorNumber& collWidth);

		uorTextDecor* 		getDefaultTextProp();
		uorTextDecor* 		getNewTextProp();
		uorBorderPropBase* 	getNewBordProp();


	public:
		uorPagePrintSetings* pagesSetings() const;
		void 	setDefaultFont(const QFont& defFont);
		int  	addFont(QString family);
		QFont*  getFontByID(const int idFont);
		QColor* getColorByID(const int idColor);
inline  long 	getChangeCount() { return m_changes;}

		uoCellJoin* getCellJoinStruct();
		QRect 		getCellJoinRect(QPoint curCell);

		void 		saveCellJoin(uoCellJoin* cellJItem);



};

} // namespace uoReport

#endif // UOREPORTDOC_H
