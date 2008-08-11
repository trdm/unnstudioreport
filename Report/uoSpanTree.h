/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/


#ifndef UOSPANTREE_H_INCLUDED
#define UOSPANTREE_H_INCLUDED


#include <QList>
#include <QObject>
#include "uoReport.h"

namespace uoReport {
///\struct uoLineSpan - специальная линия-отрезок с иерархией и некоторыми пропертями
///\brief специальная линия-отрезок с иерархией и некоторыми пропертями
struct uoLineSpan{

	int _start, _end, _level, _id;
	bool _folded;
	QList<uoLineSpan*>* _child;
	char* _name;

	public:
		uoLineSpan()
			:_start(-1)	, _end(-1)	,_level(-1)	,_folded(false),_child(0),_name(0)
			{}

		uoLineSpan(int s1, int s2)
			: _start(s1), _end(s2)	,_level(-1)	,_folded(false),_child(0),_name(0)
			{}

		int getStart() 	{ return _start;}
		int getEnd() 	{ return _end;	}
		int getLevel() 	{ return _level;	}
		bool getFolded() { return _folded;	}

		/// Получить размер отрезка
		int getSizeSpan() 	{
			return _end-_start+1;
		}

		void setId(int id) 	{_id = id;}
		int getId() 		{return _id;}

		void clearChilds() {
			uoLineSpan* spn = NULL;
			while (!_child->isEmpty()) {
				spn = _child->takeFirst();
				if(spn->_name) {
					delete spn->_name;
					spn->_name = 0;
				}
				spn->clearChilds();
				delete spn;
			}
		};

		void clear() {
			if(_name) {
				delete _name;
				_name = 0;
			}
			clearChilds();
		};

		bool isEqual(int xStart, int xEnd) {
			if (_start == xStart && _end == xEnd)
				return true;
			return false;
		};

		bool isDotInside(int xDot) {
			if (_start <= xDot && _end >= xDot)
				return true;
			return false;
		};

		/// сдвиг спана на количество точек..
		void moveTo(int xDot) {
			_start = _start + xDot;
			_end = _end + xDot;
		};

		/// Расширить диапазон спана..
		void extendTo(int xDot) {
			_end = _end + xDot;
		};

		/// Уменьшить на ХХХ, сверху или снизу
		void reduceTo(int xDot, bool fromUp = true)	{
			if (fromUp)
				_start = _start + xDot;
			else
				_end = _end - xDot;
		}


		/// Пересекают переданные ли эти координаты этот спан.
		/// Пересечение должно быть жестким, т.е.
		/// отрезки не поглощают друг-друга, а именно пересекаются
		/// 1-6 & 2-12
		bool isIntersect(int xStart, int xEnd) {
			if (isEqual(xStart,xEnd) || isContained(xStart,xEnd) || isInSide(xStart,xEnd))
				return false;
			if (
				(isDotInside(xStart) && !isDotInside(xEnd)) ||
				(isDotInside(xEnd) && !isDotInside(xStart))
				)
				return true;
			return false;
		}

		/// Анализируемый диапазон поглащается...
		bool isInSide(int xStart, int xEnd) {
			if (_start <= xStart && _end >= xEnd)
				return true;
			return false;
		}

		/// Анализируемый диапазон поглощает текущий
		bool isContained(int xStart, int xEnd) {
			if (_start >= xStart && _end <= xEnd && !(xStart==_start && xEnd==_end))
				return true;
			return false;
		}

		/// Взять количество строк над своим диапазоном.
		int getAbove(int xStart, int xEnd) {
			int rAbove = 0;
			if (isIntersect(xStart, xEnd)) {
				if (xEnd>_start)
					rAbove = _start - xStart;
				else
					rAbove = xEnd - xStart;
			} else if (xEnd < _start){
				rAbove = xEnd - xStart + 1;
			}
			return rAbove;
		}

		/// Взять количество строк пересекающего диапазона внутри своего диапазона.
		int getIntersectCount(int xStart, int xEnd) {
			int intersCnt = 0;
			if (isIntersect(xStart, xEnd)) {
				if (xStart < _start)
					intersCnt = xEnd - _start + 1;
				else
					intersCnt = qMin(xEnd,_end) - xStart + 1;
			} else if (isInSide(xStart, xEnd)) {
				intersCnt = xEnd - xStart + 1;
			} else if (isContained(xStart, xEnd)) {
				intersCnt = _end - _start + 1;
			}
			return intersCnt;
		}

		///Калькулируем вариант пересечения отрезков.
		int getIsm(int xStart, int xEnd)
		{
			int mode = ismNone;
			if (_start>xStart)
				mode = mode | ismAbove;
			if (_end < xEnd)
				mode = mode | ismBelow;
			if (isDotInside(xStart) || isDotInside(xEnd) || isContained(xStart, xEnd))
				mode = mode | ismIn;
			return mode;
		}

};

typedef QList<uoLineSpan*> spanList;
typedef QList<uoLineSpan*>::const_iterator  spanList_iterConst;
typedef QList<uoLineSpan*>::iterator  spanList_iter;


// классы сканеров для дерева.
class uoSpanTreeScan {
	public:
		uoSpanTreeScan();
		virtual ~uoSpanTreeScan() = 0;
	public:
		virtual void visitSpan(uoLineSpan* _lastAddedSpan = 0) = 0;
};

///\class uoSTS_hidePerLevel сканер для упрятывания спанов с определенным уровнем.
class uoSTS_hidePerLevel : public uoSpanTreeScan
{
	public:
		uoSTS_hidePerLevel(int perLevel, bool hide = true)
			: _perLevel(perLevel),_hMark(hide)
		{
			if (perLevel<=1) perLevel = 1;
		};
		void visitSpan(uoLineSpan* psSpn)
		{
			if (psSpn) {
				if (psSpn->_level >= _perLevel)
					psSpn->_folded = _hMark;
			}
		}
		int _perLevel;
		bool _hMark;
};



///\class uoSpanTree - класс дерева отрезков.
///\brief Класс дерева отрезков. Обслуживает иерархию отрезков посредством свох операций.
class uoSpanTree : public QObject
{
    Q_OBJECT
	public:
		uoSpanTree(QObject *parent = 0);
		~uoSpanTree();


	public:
		// Интерфейсные функции
		bool possiblyAddSpan(int start, int stop);
		bool addSpan(int start, int stop);
		int  getLevel();
		int  getSize();

		// настройки
		bool setCanOnlyOne(bool canOO, bool destroy = false);
		bool setFreezeComputeLevel(bool freeze);

		// сервис автогруппировки.
		bool groupStart(int lineStart);
		bool groupEnd(int lineEnd);

		// Обрабатываем удаление строк
		void onLineDelete(int lineStart);
		void onLinesDelete(int lineStart, int count);
		void onLinesDelete(int lineStart, int count, spanList* list);

		// Обрабатываем вставку строк
		void onLineAdd(int lineStart);
		void onLinesAdd(int lineStart, int lineCnt);
		void onLinesAdd(int lineStart, int lineEnd, spanList* list, int moveTo = 0);

		// Обрабатываем исключение строк из спанов.
		void onLineExclude(int lineStart, int lineCnt);
		int onLineExclude(int lineStart, int lineCnt, spanList* list);

		void onProcessAll(uoSpanTreeScan* scanObj, spanList* list = NULL);

		// Сохранение/восстановление.
		void onSave();
		void onLoad();

		const spanList* getSpanList(const int startLine = -1, const int endLine = -1);
		spanList* getSpanListScan(const int startLine, const int endLine, spanList* fromList = NULL, spanList* toList = NULL);

		int testClass(); // тестирование
		void clear();

	private:
		bool canAddSpanToChilds(int start, int stop, spanList* list);
		bool addSpanTo(int start, int stop, spanList* list);

		int computeLevel(spanList* list = NULL, int level = 0);
		bool listContainChild(spanList* list, int start, int stop);

		// тестирование
		int  printToDebug(spanList* list = NULL, int level = 0);
	private:
		int _maxLevel;
		int _count, _lastSpanId;
		spanList* _firstChild;
		QList<int>* _startGrpList;
		uoLineSpan* _lastAddedSpan;

		/// возможность вставлять спаны с единичным размером: 1:1, 15:15. Изначально отключена.
		bool _possibleOnlyOne;
		/// не пересчитывать уровни
		bool _freezeComputeLevel;
};

} //namespace uoReport

#endif // UOSPANTREE_H_INCLUDED
