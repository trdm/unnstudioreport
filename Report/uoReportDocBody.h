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
#include "uoReportLoader.h"
#include "uoReportDocBody.h"
#include "uoReportDoc.h"



namespace uoReport {
/**
	\struct uoEnumeratedItem - абстрактный класс нумерованного итема.
	\brief абстрактный класс нумерованного итема.
*/
struct uoEnumeratedItem
{
	virtual ~uoEnumeratedItem(){};
	virtual void setNumber(int nm) = 0;
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

		uoRptNumLine(int ln) : _line(ln), _size(-1), _hiden(false), _fixed(false)	{}
		virtual ~uoRptNumLine(){};

	void 	setNumber(int nm) {_line = nm;}
	int  	number() {return _line;}

	void 	setSize(qreal sz, bool isDef = false) {
		if (isDef)	_sizeDef = sz;
		else 		_size = sz;
	}
	qreal size(bool isDef = false) {return isDef?_sizeDef:_size;}

	bool 	hiden() {return _hiden;}
	void 	setHiden(bool hd) {_hiden = hd;}
	bool 	fixed() {return _fixed;}
	void 	setFixed(bool fx) {_fixed = fx;}

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
		qreal _size;
		qreal _sizeDef;
		bool _hiden;
		bool _fixed;
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
class uoHeaderScale	: public uoNumVector<uoRptNumLine>
{
	public:
		uoHeaderScale();
		~uoHeaderScale();

	void printToDebug();
	bool getHide(int nom );
	void setHide(int nom, bool hide);
	bool getFixed(int nom );
	void setFixed(int nom, bool fix);

	bool onStoreItems(uoReportLoader* loader);
	qreal getSize(int nom, bool isDef = false);
	bool	setSize(int nom, qreal size, bool isDef = false);

	virtual void onCreateItem(uoRptNumLine* crItem);
	virtual void onDeleteItem(uoRptNumLine* delItem);

	/// Установить дефолтный размер итема
	void setDefSize(qreal size) {		_def_size = size;	}
	/// Получить дефолтный размер итема.
	qreal getDefSizeItem() {	return _def_size;	}

	qreal _def_size;

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
		QFont* getFont(int nmFont);
		int getFontId(QString fontName);
		int addFont(QString fontName);
		int findFont(QString fontName);
		int countFonts();

		void clear();

	private:
		QList<QFont*> _fontList;
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
		uorTextDecor()	{
			resetItem();
		}
		void resetItem(){
			uorTextDecorBase::resetItem();

			m_textType 		= uoCTT_Text;
			m_vertTAlignment  = uoVA_Top;
			m_horTAlignment	= uoHA_Left;
			m_TextBehavior		= uoCTB_Auto;
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



		uoCellTextType  	m_textType;		///< Тип текста ячейки
		uoVertAlignment 	m_vertTAlignment;	///< Тип вертикального выравнивания текста.
		uoHorAlignment		m_horTAlignment;	///< Тип горизонтального выравнивания текста.
		uoCellTextBehavior 	m_TextBehavior;		///< Тип текста при превышении его длинны размера ячейки.

};
/**
	\struct uorBorderPropBase - данные о бордюре ячейки.
	\brief Содержит данные о бордюре ячейки.
*/
struct uorBorderPropBase {
	uoCellBorderType 	m_bordType[4];	///< Тип рисунка бордюра.
	int					m_bordColor;		///< Цвеет бордюра.
	uorBorderPropBase(){
		resetItem();
	}
	void resetItem()
	{
		m_bordColor = -1;
		m_bordType[0] = uoCBT_Unknown;
		m_bordType[1] = uoCBT_Unknown;
		m_bordType[2] = uoCBT_Unknown;
		m_bordType[3] = uoCBT_Unknown;
	}

};

/**
	\struct uoCellJoin данные об объединении ячеек.
	\brief Содержит данные об объединении ячеек.
	Описания режимов объединения находится uoReport.h
*/
typedef struct uoCellJoin{
	uoCellJoin()
		:m_JoinType(uoCJT_Unknown),m_row(0), m_col(0)
	{}
	~uoCellJoin()
	{}
	uoCellsJoinType m_JoinType;

	unsigned int m_row;
	unsigned int m_col;
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
		, m_height(0.0)
		, m_textProp(0)
		, m_borderProp(0)
		, m_ceelJoin(0)
		, m_textBoundary(0)
	{
		m_text			= "";
		m_textDecode 	= "";
		m_maxRowLen		= 0.0;
	}
	~uoCell()
	{}

	virtual void setNumber(int nm){		m_colNo = nm;	}
	virtual int  number() {		return m_colNo;	}
	void clear();

	bool 	provideAllProps(uoReportDoc* doc, bool needCreate = false);
	QString getText();
	QString getTextWithLineBreak(bool drawInv = false);

	void 	setText(QString text, uoReportDoc* doc);
	void 	setAlignment(const uoVertAlignment& va, const uoHorAlignment& ha, const uoCellTextBehavior& tb, uoReportDoc* doc);
	int 	getAlignment();
	void 	setMaxRowLength(qreal len, uoReportDoc* doc);
	qreal 	getMaxRowLength();

	uoHorAlignment 		getAlignmentHor();
	uoVertAlignment 	getAlignmentVer();
	uoCellTextBehavior 	getTextBehavior();
	uoCellTextType		getTextType(); //m_textType
	uorTextDecor* 		getTextProp(uoReportDoc* doc, bool needCreate = false);

	QFont*   getFont(uoReportDoc* doc, bool needCreate = false);
	int		 getFontSize();
	int		 getFontId();

	const
	QColor*  getFontColor(uoReportDoc* doc);
	int 	 getFontColorId();

	const
	QColor*  getBGColor(uoReportDoc* doc);
	int 	 getBGColorId();

	void saveTrPoint(uoTextTrPointCash* cash);
	void applyTrPoint(uoTextTrPointCash* cash, const QStringList& listStr, uoReportDoc* doc);
	bool isPartOfUnion(const int& row, const bool& basic = false) const;


	int 		m_colNo;				///< Номер колонки, к которой ячейка принадлежит.
	qreal		m_height;		///< Высота ячейки, что-бы не высчитывать формат 100 раз..


	uorTextDecor* m_textProp;
	uorBorderPropBase* m_borderProp;
	uoCellJoin*		m_ceelJoin;

	QString 	m_text; 		///< Текст содержащийся в ячейке.
	QString 	m_textDecode; 	///< Текст расшифровки.
	uoTextBoundary* 	m_textBoundary;		///< структура содержащая переносы текста.
	// чисто вспомогательные.
	qreal 		m_maxRowLen; 	///< длинна самой длинной строки в ячейке....

};


/// Строка документа. Содержит набор ячеек, в свою очередь содержащих данные.
class uoRow : public uoEnumeratedItem, public uoNumVector<uoCell>
{
	public:
		uoRow(int nom);
		virtual ~uoRow();
	public:
		void setNumber(int nm)	{	_number = nm;	}
		int  number() 			{	return _number;	}

		///\todo Необходимы поисковые механизмы, найти и получить/установить значение свойства.
		uoCell* getCell(int posX, bool needCreate = false);
		virtual void onDeleteItem(uoCell* delItem);
		virtual void onCreateItem(uoCell* crItem);
		void saveItems(uoReportLoader* loader);

		QList<int> getItemNumList();

		// Заполненные ячейки.
		int _cellFirst;	///< первая ячейка
		int _cellLast;	///< последняя ячейка

	private:
		int _number;
};


/// Коллекция строк документа
class uoRowsDoc : public uoNumVector<uoRow>
{
	public:
		uoRowsDoc();
		virtual ~uoRowsDoc();
		virtual void onDeleteItem(uoRow* delItem);
		virtual void onCreateItem(uoRow* crItem);

		void setDoc(uoReportDoc* doc){_doc = doc;};
		uoRow* getRow(int nmRow, bool needCreate = false);
		uoCell* getCell(int nmRow, int nmCol,bool needCreate = false);

		QString getText(const int posY, const int posX);
		void saveItems(uoReportLoader* loader);

		bool setText(const int posY, const int posX, QString text);
		uoReportDoc* _doc;
};



} // namespace uoReport

#endif // UOREPORTDOCBODY_H
