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

	int m_start, m_end, m_level, m_id;
	bool m_folded;
	QList<uoLineSpan*>* _child;
	QString _name;

	public:
		uoLineSpan()
			:m_start(-1)	, m_end(-1)	,m_level(-1)	,m_folded(false),_child(0)
			{}

		uoLineSpan(int s1, int s2)
			: m_start(s1), m_end(s2)	,m_level(-1)	,m_folded(false),_child(0)
			{}

		int getStart() 	{ return m_start;}
		int getEnd() 	{ return m_end;	}
		int getLevel() 	{ return m_level;	}
		int getChildCount() {
			if (!_child) {
				return 0;
			} else {
				return _child->size();
			}
		}
		bool getFolded() { return m_folded;	}
		bool isFolded() { return m_folded;	}

		/// Получить размер отрезка
		int getSizeSpan() 	{
			return m_end-m_start+1;
		}

		void setId(int id) 	{m_id = id;}
		int getId() 		{return m_id;}

		void clearChilds() {
			uoLineSpan* spn = NULL;
			while (!_child->isEmpty()) {
				spn = _child->takeFirst();
				spn->clearChilds();
				delete spn;
			}
		};

		void clear() {
			clearChilds();
		};

		bool isEqual(int xStart, int xEnd) {
			if (m_start == xStart && m_end == xEnd)
				return true;
			return false;
		};

		bool isDotInside(int xDot) {
			if (m_start <= xDot && m_end >= xDot)
				return true;
			return false;
		};

		/// сдвиг спана на количество точек..
		void moveTo(int xDot) {
			m_start = m_start + xDot;
			m_end = m_end + xDot;
		};

		/// Расширить диапазон спана..
		void extendTo(int xDot) {
			m_end = m_end + xDot;
		};

		/// Уменьшить на ХХХ, сверху или снизу
		void reduceTo(int xDot, bool fromUp = true)	{
			if (fromUp)
				m_start = m_start + xDot;
			else
				m_end = m_end - xDot;
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
			if (m_start <= xStart && m_end >= xEnd)
				return true;
			return false;
		}

		/// Анализируемый диапазон поглощает текущий
		bool isContained(int xStart, int xEnd) {
			if (m_start >= xStart && m_end <= xEnd && !(xStart==m_start && xEnd==m_end))
				return true;
			return false;
		}

		/// Взять количество строк над своим диапазоном.
		int getAbove(int xStart, int xEnd) {
			int rAbove = 0;
			if (isIntersect(xStart, xEnd)) {
				if (xEnd>m_start)
					rAbove = m_start - xStart;
				else
					rAbove = xEnd - xStart;
			} else if (xEnd < m_start){
				rAbove = xEnd - xStart + 1;
			}
			return rAbove;
		}

		/// Взять количество строк пересекающего диапазона внутри своего диапазона.
		int getIntersectCount(int xStart, int xEnd) {
			int intersCnt = 0;
			if (isIntersect(xStart, xEnd)) {
				if (xStart < m_start)
					intersCnt = xEnd - m_start + 1;
				else
					intersCnt = qMin(xEnd,m_end) - xStart + 1;
			} else if (isInSide(xStart, xEnd)) {
				intersCnt = xEnd - xStart + 1;
			} else if (isContained(xStart, xEnd)) {
				intersCnt = m_end - m_start + 1;
			}
			return intersCnt;
		}

		///Калькулируем вариант пересечения отрезков.
		int getIsm(int xStart, int xEnd)
		{
			int mode = ismNone;
			if (m_start>xStart)
				mode = mode | ismAbove;
			if (m_end < xEnd)
				mode = mode | ismBelow;
			if (isDotInside(xStart) || isDotInside(xEnd) || isContained(xStart, xEnd))
				mode = mode | ismIn;
			return mode;
		}

};

typedef QList<uoLineSpan*> spanList;
typedef QList<uoLineSpan*>::const_iterator  spanList_iterConst;
typedef QList<uoLineSpan*>::iterator  spanList_iter;


/**
	\class uoSpanTreeScan - классы сканеров для дерева.
    \brief Абстрактный класс обходчика дерева uoSpanTree
*/
class uoSpanTreeScan {
	public:
		virtual ~uoSpanTreeScan(){};
	public:
		/// возвращает: true - надо обходить полчиненные, false - не надо.
		virtual bool visitSpan(uoLineSpan* curSpan = 0) = 0;

		/// Вызывается после обходя чилдов этого спана, можно скалькулировать признак отказа от последующего сканирования.
		virtual bool visitSpanAfter(uoLineSpan* curSpan = 0) = 0;

		/// Проверка признака отказа от последующего сканирования. Если true сканирование прекращается.
		virtual bool breakProcess() {return false;};

};
/**
	Сканер для вычисления уникальности имени секции....
*/
class uoSpanScaner01 : public uoSpanTreeScan
{
	public:
		uoSpanScaner01(int perId, QString strName)
			:m_perId(perId)
			,m_findId(-1)
			,m_strName(strName)
			,m_isUnique(true)
			{}
		virtual ~uoSpanScaner01() {}

		virtual bool visitSpan(uoLineSpan* curSpan)	{
			if (!curSpan->_name.isEmpty()){
				int comp = QString::compare(curSpan->_name, m_strName, Qt::CaseInsensitive);
				if (comp == 0)
				{
					// нашли одинаковый.
					m_isUnique = false;
				}
			}
			return true;
		}
		virtual bool visitSpanAfter(uoLineSpan* curSpan){ Q_UNUSED(curSpan) return true;}
		int m_perId; 		/// Идентификатор секции с которым в данный момент работаем. -1 если не установлен
		int m_findId; 		/// Найденный идентификатор секции одинаковым именем.
		QString m_strName;
		bool m_isUnique;
};

/**
	Сканер для общих поисковых операций.
*/
class uoSpanScanerCommon : public uoSpanTreeScan
{
	public:
		uoSpanScanerCommon(uoSTScanType type)
			:m_type(type)
			,m_findSpan(0)
			,m_findCount(0)
			,m_perId(0)
			{}
		virtual ~uoSpanScanerCommon() {}

		virtual bool visitSpan(uoLineSpan* curSpan)	{
			if (!curSpan->_name.isEmpty())
			{
				switch(m_type)
				{
					case uoSTST_ById:
					{
						if (m_perId == curSpan->m_id)
						{
							++m_findCount;
							m_findSpan = curSpan;
						}
						break;
					}
					case uoSTST_ByName:
					{
						break;
					}
					case uoSTST_Unknown:
					{
						break;
					}
					default:
					{
						break;
					}
				}
			}
			return true;
		}
		virtual bool visitSpanAfter(uoLineSpan* curSpan){ return true;}

		uoSTScanType m_type;
		uoLineSpan* m_findSpan;
		int 		m_findCount;
		int 		m_perId;
		QString 	m_perName;
};


/**
	\class uoSTScan_FoldPerId сканер для вычисления списка строк/столбцов, которые надо спрятать, показать.
	\brief сканер для вычисления списка строк/столбцов, которые надо спрятать, показать.
*/
class uoSTScan_FoldPerId : public uoSpanTreeScan
{
	/*
		Скрывать/показывать надо с 2-й строки/колонки спана.
	*/
	public:
		uoSTScan_FoldPerId(int id, bool expand)
			: _perId(id),_bExpand(expand)
		{
			_bBreak = false;
			_bFound = false;
			_listProcLn 	= new QList<int>;
			_listFoldedLine = new QList<int>;
			m_foundLevel = -1;
		}
		virtual ~uoSTScan_FoldPerId(){
			delete _listFoldedLine;
		}

		/// Добавим позицЫи к обрабоке в список.
		void addToList(int start, int cnt){
			for (int i = 0; i<cnt; i++){
				_listProcLn->append(i+start);
			}
		}
		/**
			Посчитаем от обратного, соберем все сфолденные диапазоны,
			а потом пробегусь по диапазону свертки.
		*/
		void addToFoldedList(int start, int cnt){
			for (int i = 0; i<cnt; i++){
				_listFoldedLine->append(i+start);
			}
		}
		/**
			Завершающий штрих, калькулируем секции которые надо скрыть или показать,
			с пом дианазона зафолденных позиций. Если таковых нет, значит фолдим
			диапазон спана...
		*/
		void calcProcessList(uoLineSpan* curSpan)
		{
			if (_listFoldedLine->isEmpty()){
				addToList(curSpan->getStart()+1,curSpan->getSizeSpan()-1);
			} else {
				for (int i = curSpan->getStart()+1; i<=curSpan->getEnd() ; i++){
					if (!_listFoldedLine->contains(i)){
						_listProcLn->append(i);
					}
				}
			}
		}

		/// Функция посещения первого спана
		bool visitSpan(uoLineSpan* curSpan)
		{
			bool retVal = false;
			if (!curSpan)
				return retVal;
			if (curSpan->m_id == _perId){
				// Нашли узел. с него и начнем вычисления диапазона.
				_bFound = true;
				curSpan->m_folded = _bExpand;
				m_foundLevel = curSpan->getLevel();
			}

			if (_bFound)
			{
				// Найден, но есть чилды.
				if (curSpan->getLevel()>m_foundLevel){
					if (curSpan->isFolded()){
						addToFoldedList(curSpan->getStart()+1, curSpan->getSizeSpan()-1);
						return retVal;
					}
				}
			}
			return true;
		}

		/// Функция финального посещения спана. Она может установить признак завершения сканирования.
		bool visitSpanAfter(uoLineSpan* curSpan = 0){
			if (curSpan) {
				if (curSpan->m_id == _perId){
					_bFound = true;
					_bBreak = true;
					calcProcessList(curSpan);
				}
			}
			return true;
		}
		/// проверим опию завершения сканирования...
		bool breakProcess() {
			return _bBreak;
		};

		QList<int>* _listFoldedLine;
		QList<int>* _listProcLn;
		int m_foundLevel;
		int _perId;
		bool _bExpand;
		bool _bFound;
		bool _bBreak;
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
		bool addSpan(int start, int stop, bool folded = false);
		bool addSpan(int start, int stop, QString name);
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
		int  onLineExclude(int lineStart, int lineCnt);
		int  onLineExclude(int lineStart, int lineCnt, spanList* list);

		QList<int>* onGroupFold(int id, bool fold);

		uoLineSpan* getSpanById(int id);
		bool isNameUnique(int perId, const QString& name);
		bool setSectionName(int perId, const QString& name);

		void onProcessAll(uoSpanTreeScan* scanObj, spanList* list = NULL);

		// Сохранение/восстановление.
		void onSave();
		void onLoad();

		const spanList* getSpanList(const int startLine = -1, const int endLine = -1, bool foldExclude = false);
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

		bool _foldExclude; /// При выборке спанов исключать зафолденные.

		/// возможность вставлять спаны с единичным размером: 1:1, 15:15. Изначально отключена.
		bool _possibleOnlyOne;
		/// не пересчитывать уровни
		bool _freezeComputeLevel;
};

} //namespace uoReport

#endif // UOSPANTREE_H_INCLUDED
