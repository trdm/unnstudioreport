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

///\struct uoRptNumLine - нумерованная "линия" отчета (строка или столбец)
struct uoRptNumLine {
	public:

		uoRptNumLine(int ln)
			: _line(ln), _size(-1), _hiden(false), _sellected(false)
			{}

	void 	setNumber(int nm) {_line = nm;}
	int  	number() {return _line;}

	void 	setSize(rptSize sz, bool isDef = true) {
		if (isDef)	_sizeDef = sz;
		else 		_size = sz;
	}
	rptSize size(bool isDef = true) {return isDef?_sizeDef:_size;}

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
		rptSize _size, _sizeDef;///< Дефолтный размер, устанновленный в конструкторе, к которому будет возвращаться размер.
		bool _hiden, _sellected;
};

// по typename:
// http://alenacpp.blogspot.com/2006/08/typename.html

///\class uorHeaderScale Хранение и обработки длинных нумерованных структур. \n
///\brief Предназначен для хранения и обработки длинных нумерованных структур. \n
///
/// Перечень структр: Заголовки колонок и строк табличного документа, самих \n
/// строк документа, содержащих данные рядов ячеек. Данные ячеек - текст, его \n
/// форматирование, рамки, данные по их объединению и т.п.
/// (примечание) Scale - ячейка линейки, Cell - ячейка основного поля, \n чисто для изучения английского :)

class uorHeaderScale
	: public uoNumVector<uoRptNumLine>
{
	public:
		uorHeaderScale();
		~uorHeaderScale();

	void printToDebug();
	bool getHide(int nom );
	void setHide(int nom, bool hide);
	bool onStoreItems(uoReportLoader* loader);

};

///\class uoReportDocBody - тело документа
///\brief содержимое документа: строки, текст, картинки и т.п.
class uoReportDocBody
{
	public:
		uoReportDocBody();
		virtual ~uoReportDocBody();
	protected:

	public:
		void test();
		rptSize getScaleSize(uoRptHeaderType hType, int nom, bool isDef = true);
		void 	setScaleSize(uoRptHeaderType hType, int nom, rptSize size, bool isDef = true);
		void 	setScalesHide(uoRptHeaderType hType, int nmStart, int cnt = 1,  bool hide = true);
		bool 	getScaleHide(uoRptHeaderType hType, int nom);


	protected:
		uorHeaderScale* _headerV; ///< Вертикальный заголовок
		uorHeaderScale* _headerH; ///< Горизонтальный заголовок

	private:
		int _rowCount;
		int _columnCount;
};


} // namespace uoReport

#endif // UOREPORTDOCBODY_H
