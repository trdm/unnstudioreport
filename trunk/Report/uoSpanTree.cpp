/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoSpanTree.h"
#include "QDebug"

namespace uoReport {

bool sortSpanList(uoLineSpan* s1, uoLineSpan* s2)
{
     return s1->getStart() < s2->getStart();
}


/*!
	\class uoSpanTree - класс дерева отрезков.
	\brief Класс дерева отрезков. Обслуживает иерархию отрезков посредством свох операций.
	uoSpanTree - класс дерева отрезков.
	Класс дерева отрезков. Обслуживает иерархию отрезков посредством свох операций.

    служит для организации структуры группировок и секций в отчете.
    Группировка: логическое объединение групп строк или столбцов.
    Группировка "По столбцам"  включает: "ведущий" столбец и группу столбцов справа.
    Над ведушим столбцом рисуется крестик, над группой справа рисуется линия,
    над правой границей последнего столбца рисуется значек: "Правый верхний угол"

    Cервис автогруппировки. При построчном выводе отчета удобно задавать в
    програмном режиме начало и конец группировок, просто указывая номера строк.
    groupStart/groupEnd - функции сервиса автогруппировки. Номера указанные в
    функции groupStart сохраняются во внутреннем списке, при вызове функции
    groupEnd() из внутреннего списка берется последняя добавленная цифра и
    группируется с переданной.
    \code
    p->groupStart(10);
    вывод секции отчета
    p->groupEnd(20);
    \endcode

	ВАЖНОЕ
    Реализованы оперции и слоты для обработки эвентов:
    - possiblyAddSpan 	- проверка возможности добавленния спана.
    - addSpan 			- обработка добавленния спана.

    possiblyAddSpan служит для формирования динамического меню, проверка возможности
    добавления пункта меню, который скомандует "addSpan".

    СЛОТЫ
    - onLinesAdd 		- слот добавления строк.
    - onLinesDelete 	- слот удаления строк.
    - onLineExclude 	- слот исключения строк из спана.
*/

uoSpanTree::uoSpanTree(QObject *parent)
	:QObject(parent)
	,_firstChild(new spanList)
	,_startGrpList(0)
	,_lastAddedSpan(0)
	,_possibleOnlyOne(false)
	,_freezeComputeLevel(false)
	,_foldExclude(false)
{
	_lastSpanId = _maxLevel = _count = 0;
}

uoSpanTree::~uoSpanTree()
{
	clear();
	if (_firstChild)
		delete _firstChild;
	if (_startGrpList) {
		delete _startGrpList;
	}
}

void uoSpanTree::clear()
{
	if (_firstChild) {
		uoLineSpan* spn = NULL;
		while (!_firstChild->isEmpty()) {
			spn = _firstChild->takeFirst();
			spn->clear();
			delete spn;
		}
	}
	if (_startGrpList) {
		_startGrpList->clear();
	}
	_maxLevel = 0;
	_count = 0;
}

/// проверка на возможность добавления спана в определенном диапазоне
bool uoSpanTree::canAddSpanToChilds(int start, int stop, spanList* list)
{
	if (list == NULL)
		return true;
	uoLineSpan* spn = NULL;
	for (int i = 0; i<list->size(); i++) {
		spn = list->at(i);
		if (spn->isIntersect(start, stop))
			return false;
		if (spn->isInSide(start, stop)) {
			return canAddSpanToChilds(start, stop, spn->_child);
		}
	}
	return true;
}

/// общая проверка на возможность добавления спана в дерево.
bool uoSpanTree::possiblyAddSpan(int start, int stop)
{
	if ((start == stop && start <= 0) || start > stop)
		return false;
	if (!_possibleOnlyOne && start == stop)
		return false;
	if (_firstChild->isEmpty())
		return true;

	uoLineSpan* spn = NULL;
	for (int i = 0; i<_firstChild->size(); i++) {
		spn = _firstChild->at(i);
		if (spn->isIntersect(start, stop))
			return false;
		if (spn->isInSide(start, stop)) {
			return canAddSpanToChilds(start, stop, spn->_child);
		}
	}
	return true;
}

/// Проверка на существование в списке диапазонов, которые входят в переданный диапазон
/// что-бы потом перенести их в новый диапазон..
bool uoSpanTree::listContainChild(spanList* list, int start, int stop)
{
	bool resu = false;
	if (list) {
		uoLineSpan* spn = NULL;
		for (int i = 0; i<list->size(); i++) {
			spn = list->at(i);
			if (spn->isContained(start, stop)) {
				return true;
			}
		}
	}
	return resu;
}

/// Добавляем диапазон в конкретный список диапазонов
bool uoSpanTree::addSpanTo(int start, int stop, spanList* list)
{
	bool rezu = false;
	uoLineSpan* spn = NULL;
	bool needInsertHere = true;

	if (!list->isEmpty()) {
		for (int i = 0; i<list->size(); i++) {
			spn = list->at(i);
			if (spn->isInSide(start, stop)) {
				needInsertHere = false;
				return addSpanTo(start, stop, spn->_child);
			}
		}
	}
	if (list->isEmpty() || needInsertHere) {
		spn = new uoLineSpan(start, stop);
		++_lastSpanId;
		spn->setId(_lastSpanId);
		_lastAddedSpan = spn;
		spn->_child = new spanList;
		++_count;
		list->append(spn);
		rezu = true;
		if (listContainChild(list, start, stop)){
			uoLineSpan* spn2 = NULL;
			spanList_iter iterB = list->begin();
			while(iterB != list->end()) {
				spn2 = *iterB;
				if (spn2->isContained(start, stop) && spn != spn2) {
					iterB = list->erase(iterB);
					spn->_child->append(spn2);
					iterB++;
					continue;
				}
				iterB++;
			}
		}
	}
	return rezu;
}

/// Добавление диапазона.
bool uoSpanTree::addSpan(int start, int stop)
{
	bool rezu = false;
	if (!possiblyAddSpan(start, stop))
		return false;
	rezu = addSpanTo(start, stop, _firstChild);
	if(rezu && 	!_freezeComputeLevel)
		computeLevel();
	return rezu;
}

///\ onProcessAll - рекурсивная обработка дерева определенным алгоритмом.
void uoSpanTree::onProcessAll(uoSpanTreeScan* scanObj, spanList* list)
{
	if (!scanObj)
		return;
	if(scanObj->breakProcess())
		return;

	spanList* visitList = list;
	if (visitList == NULL)
		visitList = _firstChild;
	if (!visitList)
		return;
	bool needProcChild = true;
	uoLineSpan* spn = NULL;
	for (int i = 0; i<visitList->size(); i++) {
		spn = visitList->at(i);
		needProcChild = scanObj->visitSpan(spn);
		if (spn->_child && needProcChild) {
			onProcessAll(scanObj, spn->_child);
		}
		scanObj->visitSpanAfter(spn);
		if(scanObj->breakProcess())
			return;
	}
}



/// Регулирование возможности всавлять диапазоны, состоящие из одной линии: строки или столбца.
bool uoSpanTree::setCanOnlyOne(bool canOO, bool destroy)
{
	bool old = _possibleOnlyOne;
	_possibleOnlyOne = canOO;
	return old;
}

/// Заморяживание пересчета уровней при добавлении/удалении спанов, удалении строк
bool uoSpanTree::setFreezeComputeLevel(bool freeze)
{
	bool old = _freezeComputeLevel;
	_freezeComputeLevel = freeze;
	return old;
}

/// При удалении строки в контролируемом поле необходимо сдвинуть координаты спанов
void uoSpanTree::onLineDelete(int lineStart)
{
	onLinesDelete(lineStart, 1);
}

/// При удалении строки в контролируемом поле необходимо сдвинуть координаты спанов
/// Причем может удаляться диапазон строк.
void uoSpanTree::onLinesDelete(int lineStart, int count)
{
	onLinesDelete(lineStart, count, _firstChild);
	computeLevel();
}

void uoSpanTree::onLinesDelete(int lineStart, int count, spanList* list)
{
	if (!list)
		return;
	if (list->isEmpty())
		return;
	int lineEnd = lineStart + count - 1;
	int lineAbove = 0, lineIn = 0;
	bool spnProcessing = false;
	uoLineSpan* spn = NULL;
	spanList_iter iter = list->begin();
	while(iter != list->end()){
		spn = *iter;
		if (spn->getEnd() < lineStart){
			// Диапазон снизу
			iter++;
			continue;
		} else {
			lineAbove = spn->getAbove(lineStart, lineEnd);
			lineIn = spn->getIntersectCount(lineStart, lineEnd);

			/// зашита от повторной подчистки спана...
			spnProcessing = false;

			if (lineIn != 0) {
				if (spn->_child) {
					onLinesDelete(lineStart, count, spn->_child);
					spnProcessing = true;
				}

				if (spn->getSizeSpan()>lineIn) {
					spn->extendTo(-lineIn);
				} else if (spn->getSizeSpan() <= lineIn) {
					spn->clear();
					iter = list->erase(iter);
					delete spn;
					continue;
				}
			}
			if (lineAbove>0) {
				spn->moveTo(-lineAbove);
				if (spn->_child && !spnProcessing)
					onLinesDelete(lineStart, count, spn->_child);
			}

		}
		iter++;
	}
}
/// onLinesAdd - добавление линий
//  оно в принцыпе работает, только кривовато.
// 	При добавлении линий в район спанов необходимо их откорректировать
// 	Линии могут добавляться в любом порядке, в любых диапазонах
void uoSpanTree::onLineAdd(int lineStart){
	onLinesAdd(lineStart, 1);
}

void uoSpanTree::onLinesAdd(int lineStart, int lineCnt){
	if (!_firstChild)
		return;
	int lineEnd = lineStart + lineCnt - 1;

	onLinesAdd(lineStart, lineEnd, _firstChild);
}

// логика такова: если диапазон только вверху, просто сдвигаем этот спан и его чилды,
// если спан вверху и внутри сдвигаем просто сдвигаем этот спан и его чилды,
// если спан целиком только внутри РАЗдвигаем его.
// вообще логику надо доработать.
void uoSpanTree::onLinesAdd(int lineStart, int lineEnd, spanList* list, int moveTo)
{
	if (!list)
		return;
	if (list->isEmpty())
		return;
	uoLineSpan* spn = NULL;
	int linesAbove, linesIn , y, spnCnt = list->size();
	for (y = 0; y<spnCnt; y++) {
		spn = list->at(y);
		if (moveTo) {
			spn->moveTo(lineEnd - lineStart + 1);
			onLinesAdd(lineStart, lineEnd, spn->_child, moveTo);
		} else {
			linesAbove 	= spn->getAbove(lineStart, lineEnd);
			linesIn 	= spn->getIntersectCount(lineStart, lineEnd);
			if((linesAbove && linesIn) || linesAbove ) {
				moveTo = lineEnd - lineStart + 1;
				spn->moveTo(moveTo);
				onLinesAdd(lineStart, lineEnd, spn->_child, moveTo);
			} else if (linesIn) {
				spn->extendTo(lineEnd - lineStart + 1);
				onLinesAdd(lineStart, lineEnd, spn->_child, moveTo);
			}
		}
	}
}

/// Поиск спана по id.
uoLineSpan* uoSpanTree::getSpanById(int id){
	uoLineSpan* spn = NULL;
	return spn;
}

/// Обрабатываем свертку/развертку группы.
QList<int>* uoSpanTree::onGroupFold(int id, bool fold){
	// см. uoReportDescr.h логика работы свертки/развертки.
	uoSpanTreeScan* scaner = new uoSTScan_FoldPerId(id, fold);
	onProcessAll(scaner);
	QList<int>* list = ((uoSTScan_FoldPerId*)scaner)->_listProcLn;
	delete scaner;
	return list;


	return NULL;
}

/// Обрабатываем исключение строк из спанов.
/// Конкретно или удаляет спаны или делает их меньше.
void uoSpanTree::onLineExclude(int lineStart, int lineCnt)
{
	if (!(lineStart>0 && lineCnt>0))
		return;
	if (!_firstChild)
		return;
	if (_firstChild->isEmpty())
		return;
	int lineEnd;
	lineEnd = lineStart + lineCnt - 1;


	onLineExclude(lineStart, lineCnt, _firstChild);

}

/// доразработать логику, потома...
int uoSpanTree::onLineExclude(int lineStart, int lineCnt, spanList* list)
{
	int retval = 0;

	if (!list)
		return 0;
	if (list->isEmpty())
		return 0;
	int ismCur = 0, lineEnd, intersCnt;

	lineEnd = lineStart + lineCnt - 1;
	bool needDelete;

	uoLineSpan* spn = NULL;

	spanList_iter iter = list->begin();
	while (iter != list->end()) {
		spn = *iter;

		// Лучше сделаем так: первым обработается диапазон на самом нижнем уровне,
		// если надо, пользователь еще раз иницирует удаление, чем потом восстанавливать
		// если сложная структура.
		// ПС. вообще-то шагать тогда надо было так: list->end(); >> iter--; >> list->begin()
		// но пока не вижу надобности....

		if (spn->_child)
			retval = onLineExclude(lineStart, lineCnt, spn->_child);

		if (retval>0)
			break;

		needDelete = false;
		ismCur = spn->getIsm(lineStart, lineEnd);
		if (ismCur & ismIn)
		{
			if (ismCur == ismSurround) {
				needDelete = true;
			}
			else

			if (spn->isEqual(lineStart, lineEnd) || spn->isContained(lineStart, lineEnd)) 	{
				needDelete = true;
			} else

			if (ismCur == ismAboveIn || ismCur == ismInBelow || spn->getStart() == lineStart || spn->getEnd() == lineEnd) {
				intersCnt = spn->getIntersectCount(lineStart, lineEnd);
				if (ismCur == ismAboveIn || spn->getStart() == lineStart) {
					spn->reduceTo(intersCnt, true);
				} else {
					spn->reduceTo(intersCnt, false);
				}
				++retval;
			}

			if (needDelete) {
				++retval;
				spn->clear();
				iter = list->erase(iter);
				delete spn;
				continue;
			}
		}
		iter++;
	}
	return retval;
}


void uoSpanTree::onSave()
{}
void uoSpanTree::onLoad()
{}


/// groupStart/groupEnd - сервис автогруппировки.
/// Отмечаем начало группировки
bool uoSpanTree::groupStart(int lineStart)
{
	if (lineStart<=0)
		return false;
	if(!_startGrpList)
		_startGrpList = new QList<int>;

	_startGrpList->append(lineStart);
	qSort(*_startGrpList);
	return true;
}

/// Отмечаем окончание группировки
bool uoSpanTree::groupEnd(int lineEnd)
{
	if(lineEnd<=0 || !_startGrpList)
		return false;
	int line = 0;
	for(int i = 0; i<_startGrpList->count(); i++){
		line = _startGrpList->at(i);
		if(line>lineEnd)
			return false;
	}
	if (_startGrpList->isEmpty())
		return false;
	line = _startGrpList->takeLast();
	addSpan(line,lineEnd);

	return true;
}

/// перечитываем дерево и обновляем списки уровней...
int uoSpanTree::computeLevel(spanList* list, int level)
{
	spanList* curList = list;
	if (curList == NULL && level == 0) {
		curList = _firstChild;
		_maxLevel = 0; // сбросим перед пересчетом...
		_count = 0;
	}
	if (curList){
		if (!curList->isEmpty()) {
			++level;
			_maxLevel = qMax(_maxLevel, level);
			uoLineSpan* spn = NULL;
			for (int i = 0; i<curList->size(); i++) {
				spn = curList->at(i);
				++_count;
				spn->_level = level;
				if (spn->_child) {
					computeLevel(spn->_child, level);
				}
			}
			qSort(curList->begin(), curList->end(), sortSpanList);
		}
	}
	return -1;
}

/// перебираем список и выбираем спаны, соответствующие диапазону
spanList* uoSpanTree::getSpanListScan(int startLine, int endLine, spanList* fromList, spanList* toList)
{
	spanList* list = toList;
	spanList* curList = fromList;
	if (!curList)
		return list;
	if (curList->isEmpty())
		return list;
	bool truSpan = false;
	uoLineSpan* spn = NULL;
	for (int i = 0; i<curList->size(); i++) {
		spn = curList->at(i);
		truSpan = false;

		if (startLine == -1 && endLine == -1) {
			truSpan = true;

		} else if (startLine == -1 && endLine != -1) {
			truSpan = spn->getEnd() <= endLine ? true : false;

		} else if (startLine != -1 && endLine == -1) {
			truSpan = spn->getStart() >= startLine ? true : false;

		} else if (spn->getIntersectCount(startLine, endLine) != 0) {
			truSpan = true;
		}
		if (truSpan) {
			list->append(spn);
			if (spn->_child) {
				if (spn->_folded && _foldExclude) {
				} else
				if (!spn->_child->isEmpty()) {
					getSpanListScan(startLine, endLine, spn->_child, list);
				}
			}
		}
	}
	return list;
}

const spanList* uoSpanTree::getSpanList(int startLine, int endLine, bool foldExclude)
{
	bool foldExcludeOld = _foldExclude;
	_foldExclude = foldExclude;
	spanList* list = new spanList;
	getSpanListScan(startLine, endLine, _firstChild, list);
	_foldExclude = foldExcludeOld;
	return list;
}



/// вывод содержимого дерава в отладочный поток.
int uoSpanTree::printToDebug(spanList* list, int level)
{
	int retVal = 0, lCount = 0, firstCall;
	spanList* curList = list;
	if (list == NULL && level == 0) {
		curList = _firstChild;
		firstCall = 0;
	}
	level = level + 1;
	if (curList) {
		uoLineSpan* spn = NULL;
		if (!curList->isEmpty()) {
			for (int i = 0; i<curList->size(); i++) {
				lCount++;
				retVal++;
				spn = curList->at(i);
				qDebug()<<"level: "<<level<<": ("<<spn->_start<<"/"<<spn->_end<<") name: "<<spn->_name << " ID: "<< spn->_id;
				if (spn->_child) {
					retVal = retVal + printToDebug(spn->_child, level);
				}
			}
		}
	}
	return retVal;
}


/// получение максимального уровня дерева
int uoSpanTree::getLevel() {
	return _maxLevel;
}

/// Возвращает количество спанов в дереве...
int uoSpanTree::getSize(){
	return _count;
}

static void toDebugTest(bool resComp, int *testOkCnt, int *testAll, const char* str)
{
	if (resComp) {
		// если тест прошел, зачем печатать?
		++(*testOkCnt);  //qDebug()<<"success: "<<str;
	} else {
		qDebug()<<"failed: "<<str;
	}
	++(*testAll);
}

/// Запуск теста для класса.
int uoSpanTree::testClass()
{
    qDebug()<<"Start test class \"uoSpanTree\"";
    qDebug()<<"{";

    setCanOnlyOne(true);
    int nTestOk = 0, nTestAll = 0;
    bool printAll = false;

    if (true) {
		// Тестирование добавления спанов внутрь и наружу..
        qDebug()<<"Тестирование добавления спанов внутрь и наружу..";
		addSpan(1,1);
		addSpan(1,2);
		addSpan(1,6);
		addSpan(4,6);
		toDebugTest(getLevel()==3, &nTestOk, &nTestAll, "compute level = 3");
		toDebugTest(getSize()==4 , &nTestOk, &nTestAll, "getSize()==4");
		clear();
    }
    if (true) {
		// Тестирование работы с группировками..
        qDebug()<<"Тестирование работы с группировками..";
		groupStart(10);
		groupStart(11);
		groupStart(12);
		groupEnd(13);
		groupEnd(14);
		groupEnd(50);
		toDebugTest(getLevel()==3, &nTestOk, &nTestAll, "compute level = 3");
		toDebugTest(getSize()==3 , &nTestOk, &nTestAll, "getSize()==3");
		clear();
    }
    if (true) {
		// Тестирование удаления строк
        qDebug()<<"Тестирование удаления строк";
   		addSpan(1,1);
		onLineDelete(1);
		toDebugTest(getLevel()==0, &nTestOk, &nTestAll, "compute level = 0");
		toDebugTest(getSize()==0 , &nTestOk, &nTestAll, "getSize()==0");

   		addSpan(1,10);
   		addSpan(4,8);
   		addSpan(4,4);
   		addSpan(3,3);
   		addSpan(2,2);
   		addSpan(4,5);

//		const spanList* list = getSpanList(2,4);
		if (printAll) {
			printToDebug();
			onLineDelete(1); 		qDebug()<<"onLine № 1 Delete;";		printToDebug();
			onLineDelete(1); 		qDebug()<<"onLine № 1 Delete;";		printToDebug();
			onLineDelete(3); 		qDebug()<<"onLine № 3 Delete;";		printToDebug();
			onLineDelete(30); 		qDebug()<<"onLine № 30 Delete;";	printToDebug();
			onLinesDelete(4, 1); 	qDebug()<<"onLinesDelete(4, 1);";	printToDebug();
			onLinesDelete(3, 6); 	qDebug()<<"onLinesDelete(3, 6);";	printToDebug();
			onLinesDelete(1, 10); 	qDebug()<<"onLinesDelete(1, 10);";	printToDebug();
		}

    }

    if (true) {
        qDebug()<<"Тестирование анализа пересечений диапазонов";
		uoLineSpan* spn = new uoLineSpan(3,4);
		int ism_n;
		ism_n = spn->getIsm(1,1); toDebugTest(ism_n==ismAbove, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(1,2);	toDebugTest(ism_n==ismAbove, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(1,3);	toDebugTest(ism_n==ismAboveIn, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(1,4);	toDebugTest(ism_n==ismAboveIn, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(1,5);	toDebugTest(ism_n==ismSurround , &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(2,5);	toDebugTest(ism_n==ismSurround, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(3,5);	toDebugTest(ism_n==ismInBelow, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(4,5);	toDebugTest(ism_n==ismInBelow, &nTestOk, &nTestAll, "");
		ism_n = spn->getIsm(5,5);	toDebugTest(ism_n==ismBelow, &nTestOk, &nTestAll, "");
		delete spn;
    }
    if (true) {
		clear();
        qDebug()<<"Тестирование анализа вставки строк";
   		addSpan(3,7);
   		addSpan(4,4);
   		addSpan(4,6);
   		addSpan(3,3);
   		addSpan(8,8);

   		if (printAll /*|| true*/) {
			printToDebug();
			onLinesAdd(2,2); qDebug()<<"onLinesAdd(2,2);"; printToDebug();
			onLinesAdd(1,6); qDebug()<<"onLinesAdd(1,6);"; printToDebug();
			onLinesAdd(20,20); qDebug()<<"onLinesAdd(20,20);"; printToDebug();
			onLinesAdd(11,2); qDebug()<<"onLinesAdd(11,2);"; printToDebug();
			onLinesAdd(12,2); qDebug()<<"onLinesAdd(12,2);"; printToDebug();
			// вроде ок, но не совсем уверен...
   		}
	}

    if (true) {
		clear();
        qDebug()<<"Тестирование анализа исключения строк";
   		addSpan(3,7);
   		addSpan(4,4);
   		addSpan(4,6);
   		addSpan(3,3);
   		addSpan(8,8);

   		if (printAll || true) {
			printToDebug();
			onLineExclude(3,2); qDebug()<<"onLineExclude(3,2);"; printToDebug();
			onLineExclude(3,2); qDebug()<<"onLineExclude(3,2);"; printToDebug();
			onLineExclude(3,2); qDebug()<<"onLineExclude(3,2);"; printToDebug();
			onLineExclude(3,2); qDebug()<<"onLineExclude(3,2);"; printToDebug();
			onLineExclude(4,1); qDebug()<<"onLineExclude(4,1);"; printToDebug();

   		}
	}

    qDebug()<<"Test all: "<<nTestAll<<" test OK: "<< nTestOk<<" test is: "<< (nTestOk==nTestAll);

    qDebug()<<"End test class \"uoSpanTree\"";
    qDebug()<<"}";

    return  0;

}


} ///namespace uoReport
