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



namespace uoReport {
///\struct uoEnumeratedItem - абстрактный класс нумерованного итема.
struct uoEnumeratedItem
{
	virtual ~uoEnumeratedItem(){};
	virtual void setNumber(int nm) = 0;
	virtual int  number() = 0;

};

///\struct uoRptNumLine - нумерованная "линия" отчета (строка или столбец)
struct uoRptNumLine : public uoEnumeratedItem{
	public:

		uoRptNumLine(int ln)
			: _line(ln), _size(-1), _hiden(false), _sellected(false)
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

	bool 	selected() {return _sellected;}
	void 	setSelected(bool sl) {_sellected = sl;}

	private:
		int _line;
		/**
			некоторые пояснения. есть 2 размера: _size и _sizeDef
			_sizeDef - размер ячейки, как он задан в процессе конфигурирования отчета. например как дефолтная высота.
			_size    - размер ячейки, который она примет когда будет заполнена данными на этапе вывода.
			так называемый "динамический" размер. Естественно для пространственных расчетов принимается _size,
			но когда ячейка пуста, размер возвращается из _sizeDef.
		*/
		qreal _size, _sizeDef;///< Дефолтный размер, устанновленный в конструкторе, к которому будет возвращаться размер.
		bool _hiden, _sellected;
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
		ест
*/


///\struct uoTextTrPoint(uoTextTransferPoint) - точка переноса текста на новую строку.
struct uoTextTrPoint
{
	public:
		uoTextTrPoint()
			: _textTp(0),_point(-1){};
		~uoTextTrPoint();

		uoTextTrPoint* _textTp;
		int _point;
};



/**
	\struct uoCell - содержание и форматирование ячейки таблицы.
	\brief  Содержание и форматирование ячейки таблицы.

	Содержание и форматирование ячейки таблицы.
	Структура содержит текст, рамки, расшифровку, хешь шрифта.
	Данные о форматировании: о переносах строк в ячейке.

*/
struct uoCell : public uoEnumeratedItem{
	uoCell(int nom)
		:_x(nom)
		,_oldSizeCol(0)
		,_textTp(0)
	{}
	~uoCell()
	{}

	virtual void setNumber(int nm){
		_x = nm;
	}
	virtual int  number() {
		return _x;
	}

	/// Номер колонки, к которой ячейка принадлежит.
	int _x;

	/// старая ширина ячейки, до форматирования...
	qreal 		_oldSizeCol;

	/// Текст содержащийся в ячейке.
	QString 		_text;

	uoCellTextType  	_textType;		///< Тип текста ячейки
	uoVertAlignment 	_vertAlignment;	///< Тип вертикального выравнивания текста.
	uoHorAlignment		_horAlignment;	///< Тип горизонтального выравнивания текста.
	uoCellTextBehavior 	_behavior;	///< Тип текста при превышении его длинны размера ячейки.
	uoCellBorderType 	_bordType;		///< Тип рисунка бордюра.
	uoTextTrPoint* 		_textTp;			///< структура содержащая переносы текста.

};


/// Строка документа. Содержит набор ячеек, в свою очередь содержащих данные.
class uoRow : public uoEnumeratedItem, public uoNumVector<uoCell>
{
	public:
		uoRow(int nom);
		virtual ~uoRow();
	void setNumber(int nm)	{	_number = nm;	}
	int  number() 			{	return _number;	}

	///\todo Необходимы поисковые механизмы, найти и получить/установить значение свойства.
	uoCell* getCell(int posX, bool needCreate = false);
	virtual void onDeleteItem(uoCell* delItem);
	virtual void onCreateItem(uoCell* crItem);
	void saveItems(uoReportLoader* loader);


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

		uoCell* getCell(const int posY, const int posX, const bool needCreate = false);
		QString getText(const int posY, const int posX);
		void saveItems(uoReportLoader* loader);


		bool setText(const int posY, const int posX, QString text);
};



} // namespace uoReport

#endif // UOREPORTDOCBODY_H
