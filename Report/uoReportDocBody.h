/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#ifndef UOREPORTDOCBODY_H
#define UOREPORTDOCBODY_H

#include <QtCore>
#include <QLinkedList>
#include "uoReport.h"
#include "uoNumVector.h"
#include "uoNumVector2.h"
#include "uoReportLoader.h"
#include "uoReportDocBody.h"
#include "uoReportDoc.h"
#include "uoReportSelection.h"
#include "uoPainter.h"

class uoReportUndo;

namespace uoReport {
/**
	\struct uoEnumeratedItem - абстрактный класс нумерованного итема.
	\brief абстрактный класс нумерованного итема.
*/
struct uoEnumeratedItem
{
	virtual ~uoEnumeratedItem(){};
	virtual void setNumber(const int& nm) = 0;
	virtual int  number() = 0;

};

/**
	\struct uorTextDecorBase - минимальная структура с декорацией.
	\brief минимальная структура с декорацией.

	- шрифт,
	-- семейство+
	-- размер
	-- BIU
	-- цвет
	- цвет фона
	<pre>
	Далее эта структура расползется по всем декорируемым
	size_t sz2 = sizeof(uorTextDecorBase); = 4 байта, помоему вполне съедобный размер.
	использование значений для хранения данных:
	short > {...,-1,0,1000,...}
	char  > {...,-1,0,128,...}
	int:2 > {-2,-1,0,1}
	</pre>

*/
struct uorTextDecorBase
{
	short m_fontId; ///< Иднтификатор шрифта
	char m_fontSz;	///< Размер символов от -1 до 128
	int m_fontB : 2; ///< Шрифт есть Болд
	int m_fontI : 2; ///< Шрифт есть Италик
	int m_fontU : 2; ///< Шрифт есть Подчеркнутый
	short m_fontCol; ///< Цвет шрифта
	short m_BgCol; 	 ///< Цвет фона.

	uorTextDecorBase()
	: m_fontId(-1),
	m_fontSz(-1),
	m_fontB(-1),
	m_fontI(-1),
	m_fontU(-1),
	m_fontCol(-1),
	m_BgCol(-1)	{}

	void resetItem(){
		m_fontId = -1;
		m_fontSz = -1;
		m_fontB = -1;
		m_fontI = -1;
		m_fontU = -1;
		m_fontCol = -1;
		m_BgCol = -1;
	}
	void copyFrom(uorTextDecorBase* src)
	{
		m_fontId = src->m_fontId;
		m_fontSz = src->m_fontSz;
		m_fontB  = src->m_fontB;
		m_fontI  = src->m_fontI;
		m_fontU  = src->m_fontU;
		m_fontCol = src->m_fontCol;
		m_BgCol  = src->m_BgCol;
	}
	/// Равны ли проперти
	bool isEqual(uorTextDecorBase& item){
		int noEq = 0;
		if (m_fontId != item.m_fontId)
			++noEq;

		if (m_fontSz != item.m_fontSz )
			++noEq;

		if (m_fontB != item.m_fontB )
			++noEq;

		if (m_fontI != item.m_fontI )
			++noEq;

		if (m_fontU != item.m_fontU )
			++noEq;

		if (m_fontCol != item.m_fontCol )
			++noEq;

		if (m_BgCol != item.m_BgCol )
			++noEq;
		return (noEq>0)?true:false;
	}
	/**
		сольемся с переданным итемом, т.е. если у нас разные значения установим ункноун значения
		Смыслы мержинга - получить, например для палитры свойств объединенный значения.
	*/
	bool mergeItem(uorTextDecorBase& item){
		if (m_fontId != item.m_fontId && m_fontId != -1)
			m_fontId = -1;

		if (m_fontSz != item.m_fontSz && m_fontSz != -1)
			m_fontSz = -1;

		if (m_fontB != item.m_fontB && m_fontB != -1)
			m_fontB = -1;

		if (m_fontI != item.m_fontI && m_fontI != -1)
			m_fontI = -1;

		if (m_fontU != item.m_fontU && m_fontU != -1)
			m_fontU = -1;

		if (m_fontCol != item.m_fontCol && m_fontCol != -1)
			m_fontCol = -1;

		if (m_BgCol != item.m_BgCol && m_BgCol != -1)
			m_BgCol = -1;


		return isEqual(item);
	}
	bool assignItem(uorTextDecorBase& item)
	{
		if (m_fontId != item.m_fontId && item.m_fontId != -1)
			m_fontId = item.m_fontId;

		if (m_fontSz != item.m_fontSz && item.m_fontSz != -1)
			m_fontSz = item.m_fontSz;

		if (m_fontB != item.m_fontB && item.m_fontB != -1)
			m_fontB = item.m_fontB;

		if (m_fontI != item.m_fontI && item.m_fontI != -1)
			m_fontI = item.m_fontI;

		if (m_fontU != item.m_fontU && item.m_fontU != -1)
			m_fontU = item.m_fontU;

		if (m_fontCol != item.m_fontCol && item.m_fontCol != -1)
			m_fontCol = item.m_fontCol;

		if (m_BgCol != item.m_BgCol && item.m_BgCol != -1)
			m_BgCol = item.m_BgCol;

		return isEqual(item);
	}
};


/**
	\struct uoRptNumLine - нумерованная "линия" отчета (строка или столбец)
	\brief нумерованная "линия" отчета (строка или столбец)
*/
struct uoRptNumLine : public uoEnumeratedItem{
	public:

		uoRptNumLine(int ln) : _line(ln), _size(-1), _hiden(false), _fixed(false),m_textDecor(0)	{}
		virtual ~uoRptNumLine(){};

	void 	setNumber(const int& nm) {_line = nm;}
	int  	number() {return _line;}

	void 	setSize(const uorNumber& sz, bool isDef = false) {
		if (isDef)	_sizeDef = sz;
		else 		_size = sz;
	}
	uorNumber size(bool isDef = false) {return isDef?_sizeDef:_size;}

	bool 	hiden() {return _hiden;}
	void 	setHiden(const bool& hd) {_hiden = hd;}
	bool 	fixed() {return _fixed;}
	void 	setFixed(const bool& fx) {_fixed = fx;}

	private:
		int _line;
		/**
			некоторые пояснения. есть 2 размера: _size и _sizeDef
			_sizeDef - высота строки, вычисленный из максимального шрифта, который есть в строке. Для колонки смысла не имеет.
			_size    - высота строки, который она примет когда будет заполнена данными на этапе вывода.
			так называемый "динамический" размер. Естественно для пространственных расчетов принимается _size,
			но когда ячейка пуста, размер возвращается из _sizeDef.
			_size   - ЭТО ТАКЖЕ  ширина столбца
		*/
		uorNumber _size;
		uorNumber _sizeDef;
		bool _hiden;
		bool _fixed;
		uorTextDecor* m_textDecor;
};

// по typename:
// http://alenacpp.blogspot.com/2006/08/typename.html

///\class uoHeaderScale Хранение и обработки длинных нумерованных структур. \n
///\brief Предназначен для хранения и обработки длинных нумерованных структур. \n
///
/// Перечень структр: Заголовки колонок и строк табличного документа, самих \n
/// строк документа, содержащих данные рядов ячеек. Данные ячеек - текст, его \n
/// форматирование, рамки, данные по их объединению и т.п.
/// (примечание) Scale - ячейка линейки, Cell - ячейка основного поля, \n чисто для изучения английского :)
//class uoHeaderScale	: public uoNumVector2<uoRptNumLine>
class uoHeaderScale	: public uoNumVector<uoRptNumLine>
{
	public:
		uoHeaderScale();
		~uoHeaderScale();

	void printToDebug();
	bool getHide(const int& nom );
	void setHide(const int& nom, const bool& hide);
	bool getFixed(const int& nom );
	void setFixed(const int& nom, const bool& fix);

	void copyFrom(uoHeaderScale* fromSrc, int startNo = -1, int endNo = -1, int colOffset = 0);
	bool copyFrom(uoHeaderScale* fromSrc, QList<int>& listRc, int colOffset = 0);


	bool onStoreItems(uoReportLoader* loader, QList<int>* list = NULL);
	uorNumber getSize(const int& nom, bool isDef = false);
	bool	setSize(const int& nom, const uorNumber& size, bool isDef = false);

	virtual void onCreateItem(uoRptNumLine* crItem);
	virtual void onDeleteItem(uoRptNumLine* delItem);

	/// Установить дефолтный размер итема
	void setDefSize(const uorNumber& size) {		m_def_size = size;	}
	/// Получить дефолтный размер итема.
	uorNumber getDefSizeItem() {	return m_def_size;	}

	uorNumber m_def_size;

	void test();
};

/**
	\class uoReportDocFontColl - коллекция шрифтов документа.
	\brief коллекция шрифтов документа, абсорбирована для удобства выгрузок/загрузок подстановок и т.п.

	Идея вобщем-то проста. Вместо указания всех характеристик шрифтов в ячейке \n
	там просто будет торчать индекс фонта из этой коллекции и все. \n
	Думаю это позволит в будующем достаточно эффективно манипулировать со шрифтами в документах. \n

*/
class uoReportDocFontColl {
	public:
		uoReportDocFontColl();
		~uoReportDocFontColl();
	public:
		QFont* getFont(const int& nmFont);
		int getFontId(const QString& fontName);
		int addFont(const QString& fontName);
		int findFont(const QString& fontName);
		int countFonts();

		void clear();

	private:
		QVector<QFont*> _fontList;
		int m_fCount;
//		QList<QFont*> _fontList;
};


/*
	Есть определенная проблема:
	Вычисление переносов текста в ячейках таблицы при изменении некоторых
	показателей: ширина столбца, установка высоты строки пользователем и т.п.
	- Во первых, момент форматирования.
	- Во вторых, механизмы форматирования и структуры...
	Механизм форматирования:
		- проверить тип текста ячейки.
		начать форматирование только если тип: текст, тип переноса стоит uoCTB_Transfer и размер строки не фиксированный.
		- получить ширину столбца
		- высоту строки
		- строку текста.
		- отступы справа и слева.
		Начать перебор символов в строке и определение ширины каждого символа и каждого слова.
		если ширины столбца не хватает для слова, рубить его тоже.
		для этой операции необходимо хорошенько изучить информацию о шрифтах.
*/


/**
	\struct uoTextBoundary(uoTextTransferPoint) - точка переноса текста на новую строку.
	\brief точка переноса текста на новую строку.
*/
struct uoTextBoundary
{
	public:
		uoTextBoundary()	: m_textBoundary(0),_charCount(-1){};
		~uoTextBoundary(){};

		uoTextBoundary* m_textBoundary;
		int _charCount;
};

/**
	\struct uoTextTrPointCash - хешь структур uoTextBoundary.
	\brief хешь структур uoTextBoundary.
*/
struct uoTextTrPointCash
{
	public:
		uoTextTrPointCash();
		~uoTextTrPointCash();
	public:
		uoTextBoundary* getTextTrPoint();
		void savePoint(uoTextBoundary* point);
		QLinkedList<uoTextBoundary*> m_blockCash;
		void clear();
};

/**
	\struct uorTextDecor - содержит данные о тексте ячейки.
	\brief содержит данные о тексте ячейки и параметрах текста.
	Шрифт, его размер и опции, сам текст, его поведение при большой длинне..
*/
struct uorTextDecor : public uorTextDecorBase {
	public:
		uoCellTextType  	m_textType;		///< Тип текста ячейки
		uoVertAlignment 	m_vertTAlignment;	///< Тип вертикального выравнивания текста.
		uoHorAlignment 		m_horTAlignment;	///< Тип горизонтального выравнивания текста.
		uoCellTextBehavior 	m_TextBehavior;		///< Тип текста при превышении его длинны размера ячейки.
		short 				m_rotareTextAngle;	///< Угол поворота текста.
		int 				m_alignFlags;		///< Флаг для отрисовки текста (оптимизация)

	public:
		uorTextDecor()	{
			resetItem();
			m_alignFlags = 0;
		}
		void resetItem(){
			uorTextDecorBase::resetItem();

			m_textType 		= uoCTT_Text;
			m_vertTAlignment  = uoVA_Top;
			m_horTAlignment	= uoHA_Left;
			m_TextBehavior		= uoCTB_Auto;
			m_rotareTextAngle = 0;
		}
		void copyFrom(uorTextDecor* src){
			uorTextDecorBase::copyFrom(src);
			m_textType 			= src->m_textType;
			m_vertTAlignment  	= src->m_vertTAlignment;
			m_horTAlignment		= src->m_horTAlignment;
			m_TextBehavior		= src->m_TextBehavior;
		}

			/// Равны ли проперти
		bool isEqual(uorTextDecor& item){
			bool retVal = uorTextDecorBase::isEqual(item);
			if (!retVal)
				return retVal;

			int noEq = 0;
			if (m_textType 	!= item.m_textType )
				++noEq;
			if (m_vertTAlignment != item.m_vertTAlignment )
				++noEq;
			if (m_horTAlignment != item.m_horTAlignment )
				++noEq;
			if (m_TextBehavior != item.m_TextBehavior )
				++noEq;
			if (retVal && noEq>0)
				return true;
			return false;
		}
		/**
			сольемся с переданным итемом, т.е. если у нас разные значения установим ункноун значения
			Смыслы мержинга - получить, например для палитры свойств объединенный значения.
		*/
		bool mergeItem(uorTextDecor& item){
			uorTextDecorBase::mergeItem(item);

			if (m_textType 	!= item.m_textType && m_textType != uoCTT_Unknown)
				m_textType = uoCTT_Unknown;
			if (m_vertTAlignment != item.m_vertTAlignment && m_vertTAlignment != uoVA_Unknown)
				m_vertTAlignment = uoVA_Unknown;
			if (m_horTAlignment != item.m_horTAlignment && m_horTAlignment != uoHA_Unknown)
				m_horTAlignment = uoHA_Unknown;
			if (m_TextBehavior != item.m_TextBehavior && m_TextBehavior!=uoCTB_Unknown)
				m_TextBehavior = uoCTB_Unknown;
			return isEqual(item);
		}

		bool assignItem(uorTextDecor& item){
			uorTextDecorBase::assignItem(item);

			if (m_textType 	!= item.m_textType && item.m_textType != uoCTT_Unknown)
				m_textType = item.m_textType;
			if (m_vertTAlignment != item.m_vertTAlignment && item.m_vertTAlignment != uoVA_Unknown)
				m_vertTAlignment = item.m_vertTAlignment;
			if (m_horTAlignment != item.m_horTAlignment && item.m_horTAlignment != uoHA_Unknown)
				m_horTAlignment = item.m_horTAlignment;
			if (m_TextBehavior != item.m_TextBehavior && item.m_TextBehavior!=uoCTB_Unknown)
				m_TextBehavior = item.m_TextBehavior;
			return isEqual(item);
		}
};
/**
	\struct uorBorderPropBase - данные о бордюре ячейки.
	\brief Содержит данные о бордюре ячейки.

	Индексы массива m_bordType:
	0 - левый
	1 - верхний
	2 - правый
	3 - нижний бордюр

*/
struct uorBorderPropBase {
	uoCellBorderType 	m_bordType[4];		///< Тип рисунка бордюра.
	unsigned char 		m_bordSize[4];		///< Толщина бордюра.
	int					m_bordColor;		///< Цвеет бордюра.
	uorBorderPropBase(){
		resetItem();
	}
	void resetItem()
	{
		m_bordColor = -1;
		for (int i=0; i<4; i++) {
			m_bordType[i] = uoCBT_Unknown;
			m_bordSize[i] = 0;
		}
	}
	inline
	void setBorderTypes(
		const uoCellBorderType& left,
		const uoCellBorderType& top,
		const uoCellBorderType& right,
		const uoCellBorderType& bottom
	)
	{
		m_bordType[0] = left;
		m_bordType[1] = top;
		m_bordType[2] = right;
		m_bordType[3] = bottom;
	}
	inline
	void setBorderTypeAll(const uoCellBorderType& allBord )
	{
		m_bordType[0] = allBord;
		m_bordType[1] = allBord;
		m_bordType[2] = allBord;
		m_bordType[3] = allBord;
	}

	void copyFrom(uorBorderPropBase* src){
		m_bordColor = -1;
		m_bordType[0] = src->m_bordType[0];
		m_bordType[1] = src->m_bordType[1];
		m_bordType[2] = src->m_bordType[2];
		m_bordType[3] = src->m_bordType[3];
	}

	bool isEqual(const uorBorderPropBase& src){
		for (int i = 0;i<4; i++){
			if (m_bordType[i] != src.m_bordType[i])
				return false;
		}
		return (m_bordColor == src.m_bordColor);
	}

	bool mergeItem(const uorBorderPropBase& src){
		for (int i = 0;i<4; i++){
			if (m_bordType[i] != src.m_bordType[i] && m_bordType[i] != uoCBT_Unknown)
				m_bordType[i] = uoCBT_Unknown;
		}
		if (m_bordColor != src.m_bordColor && m_bordColor != -1)
			m_bordColor = src.m_bordColor;

		return isEqual(src);
	}

	bool assignItem(uorBorderPropBase& src){

		for (int i = 0;i<4; i++){
			if (m_bordType[i] != src.m_bordType[i] && src.m_bordType[i] != uoCBT_Unknown)
				m_bordType[i] = src.m_bordType[i];
		}
		if (m_bordColor != src.m_bordColor && src.m_bordColor != -1)
			m_bordColor = src.m_bordColor;

		return isEqual(src);
	}


};

/**
	\struct uoCellJoin данные об объединении ячеек.
	\brief Содержит данные об объединении ячеек.
	Описания режимов объединения находится uoReport.h
*/
struct uoCellJoin
{
	uoCellJoin():m_JoinType(uoCJT_Unknown),m_row(0), m_col(0),m_width(uorNumberNull), m_height(uorNumberNull){}
	~uoCellJoin(){}

	uoCellsJoinType m_JoinType;

	unsigned int 	m_row;		///< количество строк в объединении
	unsigned int 	m_col;		///< количество колонок в объединении
	QRect 			m_cellRect;	///< Рект номеров ячеек
	uorNumber 		m_width; 	///< Ширина ректа объединения в пикселах
	uorNumber 		m_height; 	///< Высота ректа объединения в пикселах

	void setCellRect(const int& colSt, const int& rowSt,const int& colCount,const int& rowCount)
	{
		m_cellRect = QRect(colSt, rowSt, colCount, rowCount);
	}

	void clear() {
		m_JoinType = uoCJT_Unknown;
		m_row = m_col = 0;
		m_width = m_height = uorNumberNull;

	}
};

/**
	\struct uoCell содержание и форматирование ячейки таблицы.
	\brief  Содержание и форматирование ячейки таблицы.

	Содержание и форматирование ячейки таблицы.
	Структура содержит текст, рамки, расшифровку, хешь шрифта.
	Данные о форматировании: о переносах строк в ячейке.

*/
struct uoCell : public uoEnumeratedItem{
	uoCell(int nom)
		: m_colNo(nom)
		, m_rowNo(0)
		, m_height(uorNumberNull)
		, m_textProp(0)
		, m_borderProp(0)
		, m_ceelJoin(0)
		, m_textBoundary(0)
	{
		m_text			= "";
		m_textDecode 	= "";
		m_maxRowLen		= uorNumberNull;
	}
	~uoCell()
	{}

	virtual void setNumber(const int& nm){		m_colNo = nm;	}
	virtual int  number() {		return m_colNo;	}
	void clear();
	void clearFormat(uoReportDoc* doc);

	bool 	provideAllProps(uoReportDoc* doc, bool needCreate = false);
	bool 	provideJoinProp(uoReportDoc* doc);
	QString& getText();
	QString getTextWithLineBreak(bool drawInv = false);

	void 	setText(const QString& text, uoReportDoc* doc);
	void 	setAlignment(const uoVertAlignment& va, const uoHorAlignment& ha, const uoCellTextBehavior& tb, uoReportDoc* doc);
	int 	getAlignment();
	void 	setMaxRowLength(const uorNumber& len, uoReportDoc* doc);
	void 	copyFrom(uoCell* fromSrc, uoReportDoc* doc, uoReportDoc* docSrc, int rowNo);

inline uorNumber getMaxRowLength() {return m_maxRowLen;};

	uoHorAlignment 		getAlignmentHor();
	uoVertAlignment 	getAlignmentVer();
	uoCellTextBehavior 	getTextBehavior();
	uoCellTextType		getTextType(); //m_textType
	uorTextDecor* 		getTextProp(uoReportDoc* doc, bool needCreate = false);

	QFont*   getFont(uoReportDoc* doc, bool needCreate = false);
	int		 getFontSize();
	int		 getFontId();

	inline	int		 getFontB(){if(m_textProp) return m_textProp->m_fontB; return 0; };
	inline	int		 getFontI(){if(m_textProp) return m_textProp->m_fontI; return 0; };
	inline	int		 getFontU(){if(m_textProp) return m_textProp->m_fontU; return 0; };


	const
	QColor*  getFontColor(uoReportDoc* doc);
	int 	 getFontColorId();

	const
	QColor*  getBGColor(uoReportDoc* doc);
	int 	 getBGColorId();

	void saveTrPoint(uoTextTrPointCash* cash);
	void applyTrPoint(uoTextTrPointCash* cash, const QStringList& listStr, uoReportDoc* doc);

	bool isPartOfUnion(const int& row, const bool& basic = false) const;

	bool isUnionHas() const;
	QRect getCellJoinRect();
	void setCellJoin(uoCellJoin* cellJ, uoReportDoc* doc);
	uoCellJoin* deleteCellJoin();
	uoCellJoin* cellJoin() {return m_ceelJoin;};

	uoCellsJoinType joinType() const;
	uoCellsJoinType unionType() const;
	bool isFirstOfUnionCell() const;
	int unionRow() const;
	int unionCol() const;

	bool 	skipVisitor();
	QPoint 	getFirstUnionCellPoint(const int rowNo) const;

	void drawBorder(uoPainter& painter, uorPoint& pt1, uorPoint& pt2) const;
	void drawBorder(uoPainter& painter, uorRect& rectCell) const;


	int 		m_colNo;		///< Номер колонки, к которой ячейка принадлежит.
	int 		m_rowNo;		///< Номер строки, к которой ячейка принадлежит. (используется в крайних случаях)
	uorNumber	m_height;		///< Высота ячейки, что-бы не высчитывать формат 100 раз..


	uorTextDecor* 		m_textProp;
	uorBorderPropBase* 	m_borderProp;
	uoCellJoin*			m_ceelJoin;
	QString 			m_textDecode; 	///< Текст расшифровки.

protected:
	QString 	m_text; 		///< Текст содержащийся в ячейке.
public:
	void setText(const QString& text);
	QString& text();

	uoTextBoundary* 	m_textBoundary;		///< структура содержащая переносы текста.
	// чисто вспомогательные.
	uorNumber 		m_maxRowLen; 	///< длинна самой длинной строки в ячейке....


	inline
	uorBorderPropBase* getBorderProp() {return m_borderProp ? m_borderProp : NULL; }

};

/// Структура для калькуляции ректов объединенных ячеек.
struct uoCellUnionRect
{
	uoCell* m_cell;
	uorRect 	m_rect;
	QRect 	m_uRect; ///<координаты объединения

	uoCellUnionRect(uoCell* cell)	: m_cell(cell)	{
		m_uRect.setTop(cell->m_rowNo);
		m_uRect.setLeft(cell->m_colNo);
		if (cell->m_ceelJoin){
			m_uRect.setHeight(cell->m_ceelJoin->m_row);
			m_uRect.setWidth(cell->m_ceelJoin->m_col);
		}
	};
};



/// Строка документа. Содержит набор ячеек, в свою очередь содержащих данные.
//class uoRow : public uoEnumeratedItem, public uoNumVector2<uoCell>
class uoRow : public uoEnumeratedItem, public uoNumVector<uoCell>
{
	public:
		uoRow(int nom);
		virtual ~uoRow();
	public:
		void setNumber(const int& nm)	{	m_number = nm;	}
		int  number() 			{	return m_number;	}

		void copyFrom(uoRow* fromSrc, uoReportDoc* docThere, uoReportDoc* docSrc, int startColNo = -1, int endColNo = -1,int rowOffset = 0, int colOffset = 0);

		///\todo Необходимы поисковые механизмы, найти и получить/установить значение свойства.
		uoCell* getCell(int colNo, bool needCreate = false);
		virtual void onDeleteItem(uoCell* delItem);
		virtual void onCreateItem(uoCell* crItem);
		void saveItems(uoReportLoader* loader, uoReportSelection* selection = 0);

		QList<int> getItemNumList();

		// Заполненные ячейки.
		int m_cellFirst;	///< первая ячейка
		int m_cellLast;	///< последняя ячейка
		/**
			m_lengthMaxToRight	- максимальная длина текста от первой ячейки вправо.
			m_lengthMaxToLeft 	- величина выступа текста за левый край первой ячейки.
			m_lengthFromCell	- длина от 1-й ячейки до правого края последней заполненной ячейки.

			путаница может возникнуть из-за названий (уже накололся. жара наверное.)
			m_lengthMaxToRight - для ячеек, выровненных по левому краю.
			m_lengthMaxToLeft - для ячеек, выровненных по правому краю.
		*/

		uorNumber m_lengthMaxToRight;
		uorNumber m_lengthMaxToLeft;
		uorNumber m_lengthFromCell;
		/**
			Максимальный выступ текста за пределы ячейки, нужен для расчета
			скоко цеплять колонок для полной прорисовки строки, начало которой
			в соседних ячейках.
		*/
		uorNumber 	m_lengthMaxOver;
		int 		m_unionCount; ///< количество уникальных объединений.

	private:
		int m_number;
};


/// Коллекция строк документа
//class uoRowsDoc : public uoNumVector2<uoRow>
class uoRowsDoc : public uoNumVector<uoRow>
{
	public:
		uoRowsDoc();
		virtual ~uoRowsDoc();
		virtual void onDeleteItem(uoRow* delItem);
		virtual void onCreateItem(uoRow* crItem);

		void copyFrom(uoRowsDoc* fromSrc, int startRowNo = -1, int endRowNo = -1, int startColNo = -1, int endColNo = -1, int rowOffset = 0, int colOffset = 0);
		void copyFrom(uoRowsDoc* fromSrc, uoRptHeaderType& rht, QList<int> listRc, int rowOffset = 0, int colOffset = 0);

		void setDoc(uoReportDoc* doc){m_doc = doc;};
		uoRow* getRow(int nmRow, bool needCreate = false);
		uoCell* getCell(const int& nmRow, const int& nmCol, bool needCreate = false);

		QString getText(const int rowNo, const int colNo, uorCellTextType type = uorCTT_Text);
		bool setText(const int rowNo, const int colNo, const QString& text, uorCellTextType type = uorCTT_Text);

		void saveItems(uoReportLoader* loader, uoReportSelection* selection = NULL);

		uoReportDoc* m_doc;

		// оптимизадница...
	private:
		uoRow* m_lastFRow;
		int m_lastFRowNo;
		void clearLastRow();
};



} // namespace uoReport

#endif // UOREPORTDOCBODY_H
