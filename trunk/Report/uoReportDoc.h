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
#include "uoReport.h"
#include "uoSpanTree.h"
#include "uoReportDocBody.h"
#include "uoCacheItemizer.h"



namespace uoReport {


///\class uoReportDoc - содержит данные отчета.
///\brief содержит и обслуживает данные таблицы отчета: строки, текст, картинки и т.п.
class uoReportDoc
	: public QObject
{
    Q_OBJECT
	public:
		uoReportDoc();
		virtual ~uoReportDoc();
	public:
		void clear();
		void clearFonts();
		bool addGroup(int start, int end, uoRptHeaderType ht, bool folded = false);
		bool addSection(int start, int end, uoRptHeaderType ht, QString name);

		bool possiblyAddGroup(int start, int end, uoRptHeaderType ht);
		bool possiblyAddSection(int start, int end, uoRptHeaderType ht);

		void onDeleteLine(int lineStart, int count = 1);
		const spanList* getGroupList(uoRptHeaderType rht, int start, int end);
		const spanList* getSectionList(uoRptHeaderType rht, int start, int end);

		int getGroupLevel(uoRptHeaderType ht);
		int getSectionLevel(uoRptHeaderType ht);

		void doGroupFold(int idGrop, uoRptHeaderType rht, bool fold);

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

	protected:

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

	protected:

		qreal _sizeV, _sizeV_visible;		///< Размер документа по вертикали полный и видимый.
		qreal _sizeH, _sizeH_visible;		///< Размер документа по горизонтали полный и видимый.
		/// "Видимый" - не вьювпорт, а размеры не скрытых секций

		int _rowCount, _rowCount_visible;
		int _colCount, _colCount_visible;

		int _freezEvent;	/// заморозить посылку сообщений на перерисовку

		QString 		 _docFilePath;		///< Имя файла
		uoRptStoreFormat _storeFormat;	///< Формат хранения файла отчета.

		// Группы
		uoSpanTree* _spanTreeGrH;
		uoSpanTree* _spanTreeGrV;

		// Секции.
		uoSpanTree* _spanTreeSctH;
		uoSpanTree* _spanTreeSctV;

		uoHeaderScale* _headerV; ///< Вертикальный заголовок
		uoHeaderScale* _headerH; ///< Горизонтальный заголовок
		uoTextTrPointCash* _pointBlock;
		uoCacheItemizer<uoCellJoin> m_cellJoinCash;


	protected:
		// Содержимое строк документа
		uoRowsDoc* 	_rows;		 ///< Значимое содержимое документа, содержимое ячеек документа.
	public:
		void 	setCellText(const int posY, const int posX, const QString text);
		///\todo - имплементировать... setCellFontProp
//		void 	setCellFontProp(const int idFont, const int posX, const QString text);
		void 	setCellTextAlignment(const int posY, const int posX, uoVertAlignment va,  uoHorAlignment ha, uoCellTextBehavior tb = uoCTB_Auto);
		QString getCellText(const int posY, const int posX);

		uoCell* getCell(const int posY, const int posX, bool needCreate = false);

		void doFormatDoc(int nmRow = -1, int nmForCol = -1);
		void doFormatRow(uoRow* row, int nmForCol = -1);
		void doFormatRow(int nmRow, int nmForCol = -1);
		qreal doFormatCellText(uoCell* cell, QFont* font, QFontMetricsF& fm, const qreal& collWidth );

		int getNextCellNumber(const int& rowCur, const int& colCur, const bool& ignoreHiden = true);
		QStringList splitQStringToWord(const QFontMetricsF& fm, const QString& str, const qreal& collWidth);

	protected:
		QList<uoReportCtrl*> _atachedView; 	///< Приватаченные вьювы
		QList<QObject*> 	 _atachedObj;	///< Приватаченные объекты. Документ может использоваться без вьюва, наример для заполнения его в модуле.

		long _refCounter;

	public:
		void attachView(uoReportCtrl* rCtrl, bool autoConnect = true);
		void detachedView(uoReportCtrl* rCtrl);

		void atacheObject(QObject* rObj);
		void detachedObject(QObject* rObj);

		bool isObjectAttached();
	protected:
		uoReportDocFontColl* _fontColl;
		int _defaultFontId;

	public:
		void setDefaultFont(const QFont& defFont);
		bool  addFont(QString family);
		QFont*  getFontByID(const int idFont);
		QColor*  getColorByID(const int idColor);

		uoCellTextProps* getNewTextProp();
		uoCellBordProps* getNewBordProp();

		uoCellJoin* getCellJoin();
		void 		saveCellJoin(uoCellJoin* cellJItem);



};

} // namespace uoReport

#endif // UOREPORTDOC_H
