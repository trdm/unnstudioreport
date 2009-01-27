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
#include <QFontMetricsF>
#include "uoReport.h"
#include "uoSpanTree.h"
#include "uoCacheItemizer.h"

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
class uoCacheItemizer;
struct uorTextDecorBase;


namespace uoReport {


///\class uoReportDoc - содержит данные отчета.
///\brief содержит и обслуживает данные таблицы отчета: строки, текст, картинки и т.п.
class uoReportDoc
	: public QObject
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

		void onDeleteLine(int lineStart, int count = 1);
		const spanList* getGroupList(uoRptHeaderType rht, int start, int end);
		const spanList* getSectionList(uoRptHeaderType rht, int start, int end);

		int getGroupLevel(uoRptHeaderType ht);
		int getSectionLevel(uoRptHeaderType ht);

		void doGroupFold(int idGrop, uoRptHeaderType rht, bool fold);
		bool enableCollectChanges(const bool enable);
		bool isCollectChanges() const;

		bool save();
		bool load();
		bool loadFromFile(QString path, uoRptStoreFormat stFormat);
		bool saveToFile(QString path, uoRptStoreFormat stFormat);
		bool saveOptionsIsValid();
		bool flush(uoReportLoader* loader);


		uoRptStoreFormat 	getStoreFormat(); ///< Возвращает установленный формат сохранения
		QString 			getStorePathFile();///< Возвращает установленное имя файла.
		void 				setStoreOptions(QString  filePath, uoRptStoreFormat stFormat); ///< установим опции сохранения

	public:
		void test();
		qreal 	getScalesSize(const uoRptHeaderType& hType, const int& nomStart, const int& nomEnd, const bool& ignoreHiden = false, const bool& isDef = false) const;

		qreal 	getScaleSize(uoRptHeaderType hType, int nom, bool isDef = false);
		void 	setScaleSize(uoRptHeaderType hType, int nom, qreal size, bool isDef = false);
		void 	setScaleFixedProp(uoRptHeaderType hType, int nom, bool isFixed = true);
		bool 	getScaleFixedProp(uoRptHeaderType hType, int nom);

		void 	setScalesHide(uoRptHeaderType hType, int nmStart, int cnt = 1,  bool hide = true);
		bool 	getScaleHide(uoRptHeaderType hType, int nom);

	private:
		void onAccessRowCol(int nmRow = 0, int nmCol = 0); ///< при доступе к строке или столбцу documenta...
		void onAccessRowOrCol(int nom, uoRptHeaderType rht);
		void doRowCountChange(int count, int pos = 0 );
		void doColCountChange(int count, int pos = 0 );
		void beforeAddRowOrCol(int count, uoRptHeaderType rht, int noLn = 0);

	public:
		///\todo Сделать получение и калькуляцию размеров документа.
		qreal  	getDefScaleSize(uoRptHeaderType rht);
		qreal  	getVSize(bool visible = false);
		qreal  	getHSize(bool visible = false);

		int 	getRowCount();
		int 	getColCount();

	signals:
//		void onSizeChange(qreal sizeV, qreal sizeH, int row, int col);
		void onSizeChange(qreal sizeV, qreal sizeH);
		void onSizeVisibleChangeV(qreal newSize, int newCount, qreal oldSize, int oldCnt, int pos = 0);
		void onSizeVisibleChangeH(qreal newSize, int newCount, qreal oldSize, int oldCnt, int pos = 0);

	public slots:
		void onUndo();
		void onRedo();

	protected:

		qreal m_sizeV, m_sizeV_visible;		///< Размер документа по вертикали полный и видимый.
		qreal m_sizeH, m_sizeH_visible;		///< Размер документа по горизонтали полный и видимый.
		/// "Видимый" - не вьювпорт, а размеры не скрытых секций

		int m_rowCount, m_rowCount_visible;
		int m_colCount, m_colCount_visible;

		int m_freezEvent;	/// заморозить посылку сообщений на перерисовку

		QString 		 m_docFilePath;		///< Имя файла
		uoRptStoreFormat m_storeFormat;	///< Формат хранения файла отчета.

		// Группы
		uoSpanTree* m_spanTreeGrH;
		uoSpanTree* m_spanTreeGrV;

		// Секции.
		uoSpanTree* m_spanTreeSctH;
		uoSpanTree* m_spanTreeSctV;

		uoHeaderScale* m_headerV; ///< Вертикальный заголовок
		uoHeaderScale* m_headerH; ///< Горизонтальный заголовок
		uoTextTrPointCash* m_pointBlock;

		uoReportUndo* m_undoManager;
		uoCacheItemizer<uoCellJoin> m_cellJoinCash;

	protected:
		// Содержимое строк документа
		uoRowsDoc* 	m_rows;		 ///< Значимое содержимое документа, содержимое ячеек документа.
		QList<uoReportCtrl*> m_atachedView; 	///< Приватаченные вьювы
		QList<QObject*> 	 m_atachedObj;	///< Приватаченные объекты. Документ может использоваться без вьюва, наример для заполнения его в модуле.
		long 				 m_refCounter;
		uoReportDocFontColl* m_fontColl;
		int 				 m_defaultFontId;

	public:
		void attachView(uoReportCtrl* rCtrl, bool autoConnect = true);
		void detachedView(uoReportCtrl* rCtrl);

		void atacheObject(QObject* rObj);
		void detachedObject(QObject* rObj);

		bool 	isObjectAttached();
		void 	setCellText(const int row, const int col, const QString text);

		///\todo - имплементировать... setCellFontProp
//		void 	setCellFontProp(const int idFont, const int posX, const QString text);
		void 	setCellTextAlignment(const int posY, const int posX, uoVertAlignment va,  uoHorAlignment ha, uoCellTextBehavior tb = uoCTB_Auto);
		QString getCellText(const int row, const int col);

		uoCell* getCell(const int posY, const int posX, bool needCreate = false);


		void 	doFormatDoc(int nmRow = -1, int nmForCol = -1);
		void 	doFormatRow(uoRow* row, int nmForCol = -1);
		void 	doFormatRow(int nmRow, int nmForCol = -1);
		qreal 	doFormatCellText(uoCell* cell, QFont* font, QFontMetricsF& fm, const qreal& collWidth );

		int 	getNextCellNumber(const int& rowCur, const int& colCur, const bool& ignoreHiden = true);
		QStringList splitQStringToWord(const QFontMetricsF& fm, const QString& str, const qreal& collWidth);

		uoCacheItemizer<uorTextDecor> m_TextDecorList;
		uorTextDecor* getDefaultTextProp();
		uorTextDecor* getNewTextProp();
		uorBorderPropBase* getNewBordProp();

	protected:
		void 			initTextDecorDoc(); //uorTextDecorBase* m_TextDecorDoc;
		uorTextDecor* m_TextDecorDoc;

	public:
		void 	setDefaultFont(const QFont& defFont);
		bool  	addFont(QString family);
		QFont*  getFontByID(const int idFont);
		QColor*  getColorByID(const int idColor);


		uoCellJoin* getCellJoin();
		void 		saveCellJoin(uoCellJoin* cellJItem);
		int 		m_ident;



};

} // namespace uoReport

#endif // UOREPORTDOC_H
