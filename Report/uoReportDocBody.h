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
	\struct uoRptNumLine - нумерованная "линия" отчета (строка или столбец)
	\brief нумерованная "линия" отчета (строка или столбец)
*/
struct uoRptNumLine : public uoEnumeratedItem{
	public:

		uoRptNumLine(int ln)
			: _line(ln), _size(-1), _hiden(false), _fixed(false)
			{}
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
		uoTextBoundary()	: _textBoundary(0),_charCount(-1){};
		~uoTextBoundary(){};

		uoTextBoundary* _textBoundary;
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
	\struct uoCellTextProps - содержит данные о тексте ячейки.
	\brief содержит данные о тексте ячейки и параметрах текста.
	Шрифт, его размер и опции, сам текст, его поведение при большой длинне..
*/
struct uoCellTextProps {
	public:
	uoCellTextProps()
	:_textBoundary(0)
	{
		_textType 		= uoCTT_Text;
		_vertAlignment  = uoVA_Top;
		_horAlignment	= uoHA_Left;
		_behavior		= uoCTB_Auto;
		_fontSize		= 10;
		_fontBold		= false;
		_fontItalic		= false;
		_maxTextLen		= 0.0;
	}

	QString 	_text; 	///< Текст содержащийся в ячейке.
	int 		_fontID;
	int 		_fontColID;
	int 		_fontSize;
	bool 		_fontBold;
	bool		_fontItalic;
	qreal 		_maxTextLen; ///< длинна самой длинной строки в ячейке....

	uoCellTextType  	_textType;		///< Тип текста ячейки
	uoVertAlignment 	_vertAlignment;	///< Тип вертикального выравнивания текста.
	uoHorAlignment		_horAlignment;	///< Тип горизонтального выравнивания текста.
	uoCellTextBehavior 	_behavior;		///< Тип текста при превышении его длинны размера ячейки.
	uoTextBoundary* 		_textBoundary;		///< структура содержащая переносы текста.

};
/**
	\struct uoCellBordProps - данные о бордюре ячейки.
	\brief Содержит данные о бордюре ячейки.
*/
struct uoCellBordProps {
	uoCellBorderType 	_bordType[4];	///< Тип рисунка бордюра.
	int					_bordColor;		///< Цвеет бордюра.
};

/**
	\struct uoCellJoin данные об объединении ячеек.
	\brief Содержит данные об объединении ячеек.
	Описания режимов объединения находится uoReport.h
*/
struct uoCellJoin{
	uoCellJoin()
		:m_JoinType(uoCJT_Unknown),m_Coord1(0), m_Coord2(0)
	{}
	~uoCellJoin()
	{}
	uoCellsJoinType m_JoinType;
	int m_Coord1;
	int m_Coord2;
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
		: _x(nom)
		, _bgColorID(0)
		, _textProp(0)
		, _bordProp(0)
		, m_ceelJoin(0)
	{}
	~uoCell()
	{}

	virtual void setNumber(int nm){		_x = nm;	}
	virtual int  number() {		return _x;	}
	void clear();

	QString getText();
	QString getTextWithLineBreak(bool drawInv = false);

	void 	setText(QString text, uoReportDoc* doc);
	void 	setAlignment(const uoVertAlignment& va, const uoHorAlignment& ha, const uoCellTextBehavior& tb, uoReportDoc* doc);
	int 	getAlignment();

	uoHorAlignment 		getAlignmentHor();
	uoVertAlignment 	getAlignmentVer();
	uoCellTextBehavior 	getTextBehavior();

	QFont*   getFont(uoReportDoc* doc);
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


	int 		_x;				///< Номер колонки, к которой ячейка принадлежит.
	int 		_bgColorID;		///< Индекс цвета фона ячейки...

	uoCellTextProps* _textProp;
	uoCellBordProps* _bordProp;
	uoCellJoin*		m_ceelJoin;

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
