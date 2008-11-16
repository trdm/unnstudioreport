/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "QDebug"
#include <QApplication>
#include <QFontMetrics>
#include <QRegExp>
#include "uoReportDoc.h"
#include "uoReportLoader.h"


namespace uoReport {


uoReportDoc::uoReportDoc()
//	:uoReportDocBody()
	: _spanTreeGrH(new uoSpanTree)
	, _spanTreeGrV(new uoSpanTree)
	, _spanTreeSctH(new uoSpanTree)
	, _spanTreeSctV(new uoSpanTree)
	, _headerV(new uoHeaderScale)
	, _headerH(new uoHeaderScale)
	, _rows(new uoRowsDoc)
{
	_spanTreeSctH->setCanOnlyOne(true);
	_spanTreeSctV->setCanOnlyOne(true);
	_storeFormat = uoRsf_Unknown;
	_fontColl = new uoReportDocFontColl;
	_rows->setDoc(this);


	_headerV->setDefSize(UORPT_SCALE_SIZE_DEF_VERTICAL);
	_headerH->setDefSize(UORPT_SCALE_SIZE_DEF_HORIZONTAL);

	_rowCount_visible = _rowCount 	= 0;
	_colCount_visible = _colCount 	= 0;

	_sizeV_visible = _sizeV = 0.0;	///< Размер документа по вертикали
	_sizeH_visible = _sizeH = 0.0;	///< Размер документа по горизонтали
	_freezEvent = 0;
	_refCounter = 0;
	_defaultFontId = 0;
	_pointBlock = new uoTextTrPointCash;
}

uoReportDoc::~uoReportDoc()
{
	clear();
	delete _spanTreeGrH;
	delete _spanTreeGrV;
	delete _spanTreeSctH;
	delete _spanTreeSctV;
	delete _headerV;
	delete _headerH;
	delete _fontColl;

}

/// Очистка секций.
void uoReportDoc::clear()
{
	_spanTreeGrH->clear();
	_spanTreeGrV->clear();
	_spanTreeSctH->clear();
	_spanTreeSctV->clear();

	_rowCount_visible = _rowCount 	= 0;
	_colCount_visible = _colCount 	= 0;

	_sizeV_visible = _sizeV = 0.0;	///< Размер документа по вертикали
	_sizeH_visible = _sizeH = 0.0;	///< Размер документа по горизонтали
	_fontColl->clear();
	_headerV->clear(); ///< Вертикальный заголовок
	_headerH->clear(); ///< Горизонтальный заголовок
	_rows->clear();
	_pointBlock->clear();
}

/// Очистака шрифтов. Обусловлено спецификой порядка выгрузки...
void uoReportDoc::clearFonts()
{
	_fontColl->clear();
}



/// Установить дефолтный шрифт.
void uoReportDoc::setDefaultFont(const QFont& defFont)
{
}

/// проверка возможности сформировать/вставить группу в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddGroup(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtVertical)	return _spanTreeGrV->possiblyAddSpan(start, end);
	else					return _spanTreeGrH->possiblyAddSpan(start, end);
}

/// добавление группировки в отчет.
bool uoReportDoc::addGroup(int start, int end, uoRptHeaderType ht, bool folded)
{
	bool retVal = false;
	uoSpanTree* treeGrp = NULL;

	if(ht == rhtVertical) {
		treeGrp = _spanTreeGrV;
		beforeAddRowOrCol(end - _rowCount, ht);
	} else if (ht == rhtHorizontal) {
		treeGrp = _spanTreeGrH;
		beforeAddRowOrCol(end - _colCount, ht);
	}
	retVal = treeGrp->addSpan(start, end, folded);

	return retVal;
}

/// Максимальный уровень вложения групп.
int uoReportDoc::getGroupLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == rhtVertical)
		treeGrp = _spanTreeGrV;
	else
		treeGrp = _spanTreeGrH;
	return treeGrp->getLevel();
}

/// Максимальный уровень вложения секций.
int uoReportDoc::getSectionLevel(uoRptHeaderType ht){
	uoSpanTree* treeGrp = NULL;
	if(ht == rhtVertical)	treeGrp = _spanTreeSctV;
	else					treeGrp = _spanTreeSctH;
	return treeGrp->getLevel();
}

/// Свертка/развертка группы.
void uoReportDoc::doGroupFold(int idGrop, uoRptHeaderType rht, bool fold){
	uoSpanTree* treeGrp = NULL;
	if(rht == rhtVertical)	treeGrp = _spanTreeGrV;
	else					treeGrp = _spanTreeGrH;
	++_freezEvent;
	QList<int>* lineList= treeGrp->onGroupFold(idGrop, fold);
	if (!lineList)
		return;
	if (!lineList->isEmpty()){
		int lineNo = 0;
		for (int i = 0; i<lineList->size(); i++){
			lineNo = lineList->at(i);
			setScalesHide(rht, lineNo, 1, fold);
		}
	}
	--_freezEvent;
	delete lineList;
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);
}

/// Возвращает установленный формат сохранения
uoRptStoreFormat uoReportDoc::getStoreFormat()	{
	return _storeFormat;
}

/// Возвращает установленное имя файла.
QString uoReportDoc::getStorePathFile()	{
	return _docFilePath;
}

/// установим опции сохранения
void uoReportDoc::setStoreOptions(QString  filePath, uoRptStoreFormat stFormat)	{
	_docFilePath = filePath;
	_storeFormat = stFormat;
}

/// Получить список спанов группировок по диапазону строк/столбцов. Исключая зафолденные...
const spanList* uoReportDoc::getGroupList(uoRptHeaderType rht, int start, int end)
{
	if (rht == rhtHorizontal)
		return _spanTreeGrH->getSpanList(start, end, true);
	else
		return _spanTreeGrV->getSpanList(start, end, true);
}

/// Получить список спанов СЕКЦИЙ по диапазону строк/столбцов
const spanList* uoReportDoc::getSectionList(uoRptHeaderType rht, int start, int end)
{
	if (rht == rhtHorizontal)
		return _spanTreeSctH->getSpanList(start, end, false);
	else
		return _spanTreeSctV->getSpanList(start, end, false);
}



/// проверка возможности сформировать/вставить СЕКЦИЮ в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddSection(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtHorizontal)
		return _spanTreeSctH->possiblyAddSpan(start, end);
	else
		return _spanTreeSctV->possiblyAddSpan(start, end);
}

/// вставка секции в отчет
bool uoReportDoc::addSection(int start, int end, uoRptHeaderType ht, QString name)
{
	if(ht == rhtHorizontal)	return _spanTreeSctH->addSpan(start, end, name);
	else					return _spanTreeSctV->addSpan(start, end, name);
}

void uoReportDoc::onDeleteLine(int lineStart, int count)
{
	_spanTreeSctH->onLinesDelete(lineStart, count);
}


/** Сброс документа на диск после инициализации лоадера.
	использую для обхода необходимости писать каждый раз
	деинициализацию лоадера при неудаче в выгрузке...
*/
bool uoReportDoc::flush(uoReportLoader* loader){

	bool retVal = false;

	int cntItem 	= 0;
	int i 			= 0;
	int typeHeader 	= 0;

	uoLineSpan* spn 	= NULL;
	uoSpanTree* curTree = NULL;

	uoRptHeaderType curHeaderType = rhtHorizontal;
	// Тут просто перебор документа с обращением к лоадеру..
	retVal = loader->saveDocStart(this);
	if (!retVal) return false;

	// ---------------выгрузка группировок --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= _spanTreeGrH;
			curHeaderType = rhtHorizontal;
		} else {
			curTree	= _spanTreeGrV;
			curHeaderType = rhtVertical;
		}


		cntItem = curTree->getSize();
		if (cntItem > 0) {
			retVal = loader->saveGroupsHeaderStart(cntItem, curHeaderType);
			if (!retVal) break;

			const spanList* spnList = curTree->getSpanList();
			for (i = 0; i<cntItem; i++){
				spn = spnList->at(i);
				retVal = loader->saveGroupsItem(spn);
				if (!retVal) break;
			}

			delete spnList;
			spnList = NULL;

			retVal = loader->saveGroupsHeaderEnd(curHeaderType);
			if (!retVal) break;
		}
	}
	if (!retVal) return false;

	// ---------------выгрузка секций --------------
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			curTree	= _spanTreeSctH;
			curHeaderType = rhtHorizontal;
		} else {
			curTree	= _spanTreeSctV;
			curHeaderType = rhtVertical;
		}


		cntItem = curTree->getSize();
		if (cntItem > 0) {
			retVal = loader->saveSectionHeaderStart(cntItem, curHeaderType);
			if (!retVal) break;

			const spanList* spnList = curTree->getSpanList();
			for (i = 0; i<cntItem; i++){
				spn = spnList->at(i);
				retVal = loader->saveSectionItem(spn);
				if (!retVal) break;
			}

			delete spnList;
			spnList = NULL;

			retVal = loader->saveSectionHeaderEnd(curHeaderType);
			if (!retVal) break;
		}
	}
	if (!retVal) return false;

	uoHeaderScale* headerScale = NULL;

	// ---------------выгрузка размеров строк/столбцов --------------
	// Тут же надо делать выгрузку соодержимого ячеек.
	for (typeHeader = 1; typeHeader<=2; typeHeader++) {
		if(typeHeader == 1){
			headerScale	= _headerH;
			curHeaderType = rhtHorizontal;
		} else {
			headerScale	= _headerV;
			curHeaderType = rhtVertical;
		}

		cntItem = headerScale->getCountItem();
		if (cntItem > 0) {
			retVal = loader->saveScaleHeaderStart(cntItem, curHeaderType);
			if (!retVal) break;

			headerScale->onStoreItems(loader);

			retVal = loader->saveScaleHeaderEnd(curHeaderType);
			if (!retVal) break;
		}
	}

	//--------- выгрузка содержания строк -------------------
	int rowCount = _rows->getCountItem();
	if (rowCount>0) {
		loader->saveRowsStart(rowCount);
		_rows->saveItems(loader);
		loader->saveRowsEnd();
	}
	if (_fontColl->countFonts()>0){
		QFont* font = NULL;
		loader->saveFontStart(_fontColl->countFonts());
		for (i = 0; i< _fontColl->countFonts(); i++){
			font = _fontColl->getFont(i);
			loader->saveFont(font, i);
		}
		loader->saveFontEnd();


	}

	if (!retVal) return false;
	retVal = loader->saveDocEnd(this);
	return retVal;
}

/// Сохранение отчета. Инициализируем лоадер, вызываем сохранение...
bool uoReportDoc::saveToFile(QString path, uoRptStoreFormat stFormat)
{
	bool retVal = false;
	if (stFormat == uoRsf_Unknown) {
		qWarning() << tr("no define store format");
		return false;
	}
	uoReportLoader* loader = uoReportLoader::getLoader(stFormat);
	if (!loader) {
		qWarning() << tr("Can not create loader");		return false;
	}

	loader->setFileName(path);
	if (!loader->init(false)){
		qWarning() << loader->getLastError();
	} else {
		flush(loader);
	}
	loader->finalize();
	delete loader;

	return retVal;
}

/// Проверка на валидность опций записи: имя файла и тип документа.
bool uoReportDoc::saveOptionsIsValid(){
	if (_docFilePath.isEmpty() || _storeFormat == uoRsf_Unknown) {
		return false;
	}
	return true;

}

/// Сохранение отчета.
bool uoReportDoc::save(){
	if (saveOptionsIsValid()) {
		return saveToFile(_docFilePath, _storeFormat);
	}
	return false;
}

/// Считывание отчета.
bool uoReportDoc::load()
{
	if (saveOptionsIsValid()) {
		return loadFromFile(_docFilePath, _storeFormat);
	}
	return false;
}

bool uoReportDoc::loadFromFile(QString path, uoRptStoreFormat stFormat)
{
	bool retVal = false;
	if (stFormat == uoRsf_Unknown) {
		qWarning() << tr("no define store format");
		return false;
	}
	uoReportLoader* loader = uoReportLoader::getLoader(stFormat);
	if (!loader) {
		qWarning() << tr("Can not create loader");		return false;
	}

	loader->setFileName(path);
	if (!loader->init(true)){
		qWarning() << loader->getLastError();
	} else {
		loader->load(this);
	}
	loader->finalize();
	delete loader;

	return retVal;

}



/// Возвращаем дефолтный размер строки или колонки
qreal  	uoReportDoc::getDefScaleSize(uoRptHeaderType rht)
{
	if (rht == rhtVertical){
		return _headerV->getDefSizeItem();
	} else if (rht == rhtHorizontal) {
		return _headerH->getDefSizeItem();
	}
	return 0.0;
}

/// Возвращаем высоту документа полную (по умолчанию) или видимую.
qreal  	uoReportDoc::getVSize(bool visible){
	if (visible)
		return _sizeV_visible;
	else
		return _sizeV;
}

/// Возвращаем длину документа полную (по умолчанию) или видимую.
qreal  	uoReportDoc::getHSize(bool visible){
	if (visible)
		return _sizeH_visible;
	else
		return _sizeH;
}

/// Возвращаем количество строк
int 	uoReportDoc::getRowCount(){	return _rowCount;}

/// Возвращаем количество столбцов
int 	uoReportDoc::getColCount(){	return _colCount;}


/// перед добавлением строк или столбцов. Необходимо что-бы посчитать длину/ширину дока.
void uoReportDoc::beforeAddRowOrCol(int count, uoRptHeaderType rht, int noLn)
{
	if (count <= 0)
		return;
	int oldCnt = 0;

	uoHeaderScale* header = NULL;
	if (rht == rhtVertical) {
		header = _headerV;
		oldCnt = _rowCount;
	} else {
		header = _headerH;
		oldCnt = _colCount;
	}
	qreal itemSize 	 = 0.0;
	qreal addSize 	 = 0.0;
	qreal addSizeVis = 0.0;
	for (int i = 1; i<=count; i++){
		itemSize = header->getSize(oldCnt + i);
		addSize = addSize + itemSize;
		if (!header->getHide(oldCnt + i)){
			addSizeVis = addSizeVis + itemSize;
		}
	}


	if (rht == rhtVertical) {
		_rowCount 		= _rowCount + count;
		_sizeV 			= _sizeV + addSize;
		_sizeV_visible 	= _sizeV_visible + addSizeVis;
	} else {
		_colCount 		= _colCount + count;
		_sizeH 			= _sizeH + addSize;
		_sizeH_visible 	= _sizeH_visible + addSizeVis;
	}
	///\todo 1 А вот тут нужен сигнал на изменение размеров документа....
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);
}

/// Изменить количество строк в документе
void uoReportDoc::doRowCountChange(int count, int pos)
{
	if (count == 0)
		return;
	qreal oldSize = _sizeV;
	qreal oldSizeVis = _sizeV_visible;

	_rowCount = _rowCount + count;
	_sizeV = oldSize + count * getDefScaleSize(rhtVertical);
	_sizeV_visible = oldSizeVis + count * getDefScaleSize(rhtVertical);
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);
}

/// Изменить количество столбцов в документе
void uoReportDoc::doColCountChange(int count, int pos )
{
	if (count == 0)
		return;
	qreal oldSize = _sizeH;
	qreal oldSizeVis = _sizeH_visible;

	_colCount = _colCount + count;
	_sizeH = oldSize + count * getDefScaleSize(rhtHorizontal);
	_sizeH_visible = oldSizeVis + count * getDefScaleSize(rhtHorizontal);
	if (_freezEvent == 0)
		emit onSizeChange(_sizeV_visible, _sizeH_visible);

}


/// При доступе к строке или ячейке на запись. Служит для определения длины/ширины документа.
void uoReportDoc::onAccessRowOrCol(int nom, uoRptHeaderType rht)
{
	int cnt = 0;
	if (rht == rhtHorizontal) // Колонка
	{
		if (_colCount < nom) {
			cnt = nom - _colCount;
			doColCountChange(cnt, _colCount+1);
		}
	} else if (rht == rhtVertical) {		// строка
		if (_rowCount < nom) {
			cnt = nom - _rowCount;
			doRowCountChange(cnt, _rowCount+1);
		}
	}
}

void uoReportDoc::onAccessRowCol(int nmRow, int nmCol)
{
	if (nmRow > 0)		onAccessRowOrCol(nmRow, rhtVertical);
	if (nmCol > 0)		onAccessRowOrCol(nmCol, rhtHorizontal);
}


/// Вернуть размер ячейки
qreal uoReportDoc::getScaleSize(uoRptHeaderType hType, int nom, bool isDef)
{
	if (hType == rhtVertical)
		return _headerV->getSize(nom, isDef);
	else
		return _headerH->getSize(nom, isDef);
}

/// Вернуть размер диапазона ячек
qreal uoReportDoc::getScalesSize(const uoRptHeaderType& hType, const int& nomStart, const int& nomEnd, const bool& ignoreHiden, const bool& isDef) const
{
	uoHeaderScale* pHeader = NULL;
	if (hType == rhtVertical) {
		pHeader = _headerV;
	} else {
		pHeader = _headerH;
	}
	qreal retVal = 0.0;
	int nmStart = qMin(nomStart,nomEnd), nmEnd = qMax(nomStart,nomEnd), i = 0;
	for (i = nmStart; i<=nmEnd; i++){
		if (pHeader->getHide(i) && ignoreHiden)
			continue;
		retVal = retVal + pHeader->getSize(i, isDef);
	}
	return retVal;
}

/// Установить размер ячейки
void uoReportDoc::setScaleSize(uoRptHeaderType hType, int nom, qreal size, bool isDef){
	bool scVisible = true;
	qreal oldSizeItem = 0.0, oldSize = 0.0;
	if (hType == rhtVertical) {
		oldSizeItem = _headerV->getSize(nom, isDef);
		scVisible = _headerV->getHide(nom);
		_headerV->setSize(nom, size, isDef);
		_sizeV = _sizeV - oldSizeItem + size;
		if (!scVisible) {
			oldSize = _sizeV_visible;
			_sizeV_visible = _sizeV_visible - oldSizeItem + size;
			if (nom <= _rowCount)
				emit onSizeVisibleChangeV(_sizeV_visible, _rowCount, oldSize, _rowCount, nom);
		}
	}
	else {
		oldSizeItem= _headerH->getSize(nom, isDef);
		_headerH->setSize(nom, size, isDef);
		_sizeH = _sizeH - oldSizeItem + size;
		scVisible = _headerH->getHide(nom);
		if (!scVisible) {
			oldSize = _sizeH_visible;
			_sizeH_visible = _sizeH_visible - oldSizeItem + size;
			if (nom <= _colCount)
				emit onSizeVisibleChangeH(_sizeH_visible, _colCount, oldSize, _colCount, nom);
		}
	}
	onAccessRowOrCol(nom-1, hType); // последнюю ячейку не трогаем, т.к. её размер устанавливается...
}

/**
	Установка свойства, что строка иммет фиксированную высоту,
	т.е. при изменении текста её размер не меняется
	Примечание. бесполезно для столбцов, они не будут подгоняться
	"под размер" текста. Ну по крайней мере пока...
*/
void uoReportDoc::setScaleFixedProp(uoRptHeaderType hType, int nom, bool isFixed)
{
	if (hType == rhtVertical) {
		_headerV->setFixed(nom, isFixed);
	}
	/// для колонок собственно безсмысленно...
}

bool uoReportDoc::getScaleFixedProp(uoRptHeaderType hType, int nom)
{
	if (hType == rhtVertical) {
		return _headerV->getFixed(nom);
	}
	return false;
}


/// Прячем/Показываем диапазон ячеек...
void uoReportDoc::setScalesHide(uoRptHeaderType hType, int nmStart, int cnt,  bool hide){
	uoHeaderScale* header = NULL;
	if (hType == rhtVertical) {
		header = _headerV;
	} else {
		header = _headerH;
	}
	qreal szAdd = 0.0;
	for (int i = 0; i<cnt; i++)	{
		szAdd = szAdd + header->getSize(nmStart + i);
		header->setHide(nmStart + i, hide);
	}
	qreal oldSize = 0.0;
	szAdd = szAdd * ( hide ? -1 : 1);
	if (hType == rhtVertical){
		oldSize = _sizeV_visible;
		_sizeV_visible = _sizeV_visible + szAdd;
		if (_freezEvent == 0)
			emit onSizeChange(_sizeV_visible, _sizeH_visible);
	} else {
		oldSize = _sizeH_visible;
		_sizeH_visible = _sizeH_visible + szAdd;
		if (_freezEvent == 0)
			emit onSizeChange(_sizeV_visible, _sizeH_visible);
	}

	onAccessRowOrCol(nmStart + cnt - 1, hType);
}

/// Нужно получить следующую занятую ячейку в строке отноительно предыдущей...
int uoReportDoc::getNextCellNumber(const int& rowCur, const int& colCur, const bool& ignoreHiden)
{
	int retVal = -1;

	uoRow* row = _rows->getRow(rowCur);
	if (row) {
		row->getNextItemNom(colCur);
	}

	return retVal;
}

/// пошинковать строку на слова не превышающие длинны collWidth
QStringList uoReportDoc::splitQStringToWord(const QFontMetricsF& fm, const QString& str, const qreal& collWidth)
{
	QStringList list;
	QString curStr, curWord;
	QChar qchar;
	qreal lenStr = 0.0, lenCurW = 0.0;

	QStringList listW = str.split(QRegExp("\\b"));
	for (int i = 0; i<listW.count(); i++)
	{
		curWord = listW.at(i);
		if (curWord.isEmpty())
			continue;

		lenCurW = fm.width(curWord);
		if ((lenStr + lenCurW)>collWidth){
			if (!curStr.isEmpty()) {
				list.append(curStr);
				curStr = "";
				lenStr = 0.0;
			}
			if (lenCurW > collWidth) {
				// а вдруг длинна слова больше чем общая ширина?
				QString ww;
				QChar chr;
				int ii = 0; curWord.length();
				while(ii<curWord.length())
				{
					chr = curWord.at(ii);
					if ((fm.width(chr) + fm.width(ww))>collWidth){
						list.append(ww);
						ww = "";
					}
					ww.append(chr);
					ii = ii + 1;
				}
				curStr.append(ww);
			} else {
				curStr.append(curWord);
			}
		} else {
			curStr.append(curWord);
		}
		lenStr = fm.width(curStr);
	}
	if (!curStr.isEmpty())
		list.append(curStr);
	return list;
}


/// Форматирование текста ячейки, одновременно расчет её высоты..
qreal uoReportDoc::doFormatCellText(uoCell* cell, QFont* font, QFontMetricsF& fm, const qreal& collWidth)
{
	qreal cellHeight = 0.0;
	qreal collWidthThis = collWidth - UORPT_STANDART_OFFSET_TEXT*2; // 2 офсета, справ и слева.


	// задачка не из простых...
	QString cellStr = cell->getText();
	if (cell->_textProp){
		_pointBlock->savePoint(cell->_textProp->_textBoundary);
		cell->_textProp->_textBoundary = NULL;
		cell->_textProp->m_maxRowLen = 0.0;
	}

	if (cellStr.isEmpty())
		return cellHeight;


	cellStr = cellStr.replace('\t', " ");
	uoCellTextBehavior 	tb = cell->getTextBehavior();
	/*
		походу очень плохая идея рубить текст и оставлять его порубленным в ячейке.
		лучше уж определить масив сплитеров и рубить его при выводе...
	*/

	QChar chrLS = '\n';
	int pos = cellStr.indexOf(chrLS, 0);
	QStringList strListReal;
	int listSz = 0;

	qreal fullLength = fm.width(cellStr); // нужно проверить, влезает ли текст в сейку..
	QString nextStr, curStr;
	if (pos != -1 || tb == uoCTB_Transfer){
		// Значит надо рубить
		QStringList strList = cellStr.split(chrLS);
		listSz = strList.count();

		for (int i = 0; i<strList.count(); i++) {
			nextStr = strList.at(i);

			if ((i+1)<listSz){
				nextStr = nextStr + "\n";
			}
			if (tb == uoCTB_Transfer){
				if (fm.width(cellStr) > collWidthThis){
					// надо сплитануть по словам: по табу, пробелу, знакам препинания и т.п.
					// незабыть стандартный офсет, служащий для антиприлипания строки.
					QStringList listOfWord = splitQStringToWord(fm, nextStr, collWidthThis);
					strListReal << listOfWord;
				} else {
					strListReal << nextStr;
				}
			} else {
				strListReal << nextStr;
			}
		}
	} else {
		strListReal << cellStr;
	}

	qreal maxRowLength = 0.0;
	for (int i = 0; i < strListReal.size(); ++i){
		nextStr = strListReal.at(i);
		maxRowLength = qMax(maxRowLength, fm.width(nextStr));
	}

	cell->setMaxRowLength(maxRowLength, this);
	cell->applyTrPoint(_pointBlock, strListReal, this);
	cellHeight = fm.height() * strListReal.size();
	return cellHeight;
}

/**
	Форматирование строки: \n
		- вычисление максимальной ВЫСОТЫ строки.
		- вычисление переносов \n
		- вычисление ВЫСОТЫ, переносов и т.п. \n
*/
void uoReportDoc::doFormatRow(uoRow* row, int nmForCol /* = -1*/ )
{
	/*
		Задача по форматированию:
		- вычисление максимальной высоты строки	если её размер не зафиксирован.
		- вычисление переносов текста в ячейках.
	*/
	if (!row)
		return;
	QList<int> cellsNumbers = row->getItemNumList();
	if (cellsNumbers.isEmpty())	{
		return;
	}
	uoCell* cell = NULL;
	QString cellText;
	QFont* font;
	int nmRow = row->number();
	int cellPrevNo = -1;
	int cellCurNo = -1;
	int cellNextNo = -1;

	int cellCounts = cellsNumbers.size();
	qreal collSize = 0.0; //getScaleSize(rhtVertical

	qreal rowMinSize = _headerV->getDefSizeItem();
	qreal rowRealSize = rowMinSize;
	qreal cellHeight = 0;
	uoCellTextBehavior textBehav = uoCTB_Auto;
	uoHorAlignment textHorAl = uoHA_Left;

	bool collBeFormated = false;

	if(nmForCol<=0)
		collBeFormated = true;

	for (int i = 0; i<cellCounts; i++){
		cellCurNo = cellsNumbers.at(i);
		cell = row->getItem(cellCurNo, false);
		rowRealSize = qMax(cell->m_height,rowRealSize);
		rowRealSize = qMax(rowRealSize, rowMinSize);


		if (nmForCol != -1 && nmForCol != cellCurNo) {
			// нужно еще проверить ячейки на объединение...
			continue;
		}
		cell->m_height = 0.0;

		collBeFormated = true;
		cellNextNo = cellCurNo;
		if (cellCounts>i+1){
			cellNextNo = cellsNumbers.at(i+1);
		}

		if (cellPrevNo == -1 && cellCurNo>1)
			cellPrevNo = 1;

		collSize = getScaleSize(rhtHorizontal,cellCurNo);

		textBehav = cell->getTextBehavior();
		textHorAl = cell->getAlignmentHor();

		font = cell->getFont(this);
		cellText = cell->getText();

		if (font && !cellText.isEmpty()) {

			// тут уже можно хотя бы посчитать минимальную высоту строки. что и делаем...
			QFontMetricsF fm(*font);
			rowMinSize = qMax(rowMinSize, (qreal)fm.height());

			// а тут можно переформатировать текст
			if (cellNextNo != -1 && textBehav == uoCTB_Auto) {
				if (textHorAl == uoHA_Center){
					if ((cellPrevNo != -1) && (cellNextNo != -1)) {
						collSize = getScalesSize(rhtHorizontal, qMin(cellPrevNo+1, cellCurNo), qMax(cellCurNo,cellNextNo-1), true);
					} else if (cellNextNo != -1){
						collSize = getScalesSize(rhtHorizontal, cellCurNo, qMax(cellCurNo,cellNextNo-1), true);
					} else if (cellPrevNo != -1){
						collSize = getScalesSize(rhtHorizontal, qMin(cellPrevNo+1, cellCurNo), cellCurNo, true);
					}
				} else if (textHorAl == uoHA_Left) {
					if (cellNextNo != -1) {
						collSize = getScalesSize(rhtHorizontal, cellCurNo, qMax(cellCurNo,cellNextNo-1), true);
					} else {
						collSize = getScalesSize(rhtHorizontal, cellCurNo, cellCurNo, true);
					}
				} else if (textHorAl == uoHA_Right) {
					if (cellPrevNo != -1) {
						collSize = getScalesSize(rhtHorizontal, qMin(cellPrevNo+1, cellCurNo), cellCurNo, true);
					} else {
						collSize = getScalesSize(rhtHorizontal, cellCurNo, cellCurNo, true);
					}
				}
			}
			cellHeight = doFormatCellText(cell, font, fm, collSize);
			cell->m_height = cellHeight;

			rowRealSize = qMax(cellHeight,rowRealSize);
			rowRealSize = qMax(rowRealSize, rowMinSize);
		}


		/*
			есть несколько вариантов выравнивания текста по горизонтали:
			влево, [<=   ]
			вправо, [   =>]
			центр, [  <=>  ]
			а так же признак "по выделенным столбцам". влево вправо или по центру.
			[  <=  ][  =  ][  =  ][  =  ][  =>  ]
			нужно вычислить как именно выравнивается текст и в какие рамки он может уместиться..
			если он достаточно длинен, и имеет признак выравнивания авто,
			то его можно напечатать и на соседних ячейках, не прорисовывая их..
			ДЛЯ НАЧАЛА - попробуем пересчитать высоту строки при изменениии ширины столбца.
		*/


		cellPrevNo = cellCurNo; // должно быть вконце....
	}
	if (collBeFormated && !_headerV->getFixed(nmRow)) {
		_headerV->setSize(nmRow,rowRealSize);
	}

	///\todo а вот тут надо гавкнуть, что-бы перещитали ректы, если документ подсоединен ко вьюву...

}

/// Форматирование строки: вычисление ВЫСОТЫ, переносов и т.п.
void uoReportDoc::doFormatRow(int nmRow, int nmForCol)
{

	uoRow* row = _rows->getRow(nmRow, false);
	if (!row)
		return;
	doFormatRow(row, nmForCol);
}

/// Форматирование документа..
void uoReportDoc::doFormatDoc(int nmRow /*= -1*/, int nmForCol /*= -1*/)
{
	if (_rows->getCountItem() <=0)
		return;
	uoRow* row = NULL;
	int curRowNum = _rows->getMinNo();
	if (nmRow > 0){
		uoRow* row = _rows->getItem(nmRow, false);
		doFormatRow(row, nmForCol);
	} else {
		uoRow* row = _rows->getItem(curRowNum, false);
		while(row){
			doFormatRow(row, nmForCol);
			row = _rows->getNextItem(curRowNum);
			if (row){
				curRowNum = row->number();
			}
		}
	}
}

/// Установить текст в ячейку
void uoReportDoc::setCellText(const int posY, const int posX, const QString text)
{
	_rows->setText(posY, posX, text);	// форматирование? угу.
	onAccessRowOrCol(posY, rhtVertical);
	onAccessRowOrCol(posX, rhtHorizontal);
	doFormatRow(posY, -1);
}

/// Установить выравнивание текста в ячейке
void uoReportDoc::setCellTextAlignment(const int posY, const int posX, uoVertAlignment va,  uoHorAlignment ha, uoCellTextBehavior tb)
{
	uoCell* cell = _rows->getCell(posY, posX, true);
	if (cell){
		cell->setAlignment(va,  ha, tb, this);
		doFormatRow(posY, posX);
	}
}

QString uoReportDoc::getCellText(const int posY, const int posX){
	return 	_rows->getText(posY, posX);
}

uoCell* uoReportDoc::getCell(const int posY, const int posX, bool needCreate){
	uoCell* retCell = _rows->getCell(posY, posX, needCreate);
	return retCell;
}


bool uoReportDoc::getScaleHide(uoRptHeaderType hType, int nom){
	if (hType == rhtVertical)
		return _headerV->getHide(nom);
	else
		return _headerH->getHide(nom);
}

/// присоединим вьюв..
void uoReportDoc::attachView(uoReportCtrl* rCtrl, bool autoConnect){
	bool found = false;
	if (!_atachedView.isEmpty()) {
		 found = _atachedView.contains(rCtrl);
	}
	if (!found) {
		++_refCounter;
		_atachedView.append(rCtrl);
		/// надо его законнектить к сигналам. Тока сигналы до ума довести.....
		if (autoConnect) {
	//		connect(
		}

	}
}

/// Отсоединяемся от вьюва.
void uoReportDoc::detachedView(uoReportCtrl* rCtrl)
{
	if (!_atachedView.isEmpty()) {
		if (_atachedView.contains(rCtrl)) {
			int pos = 0; _atachedView.indexOf(rCtrl);
			while((pos = _atachedView.indexOf(rCtrl))>0){
				_atachedView.removeAt(pos);
			}
			--_refCounter;
		}
	}
}

/// Объект используется/не используется. Если используется, удалять нельзя...
bool uoReportDoc::isObjectAttached()
{
	if (_refCounter > 0 || !_atachedView.isEmpty() || !_atachedObj.isEmpty())
		return true;
	return false;
}

/// Получить шрифт по ID
QFont*  uoReportDoc::getFontByID(const int idFont)
{
	if (idFont<0)
		return NULL;

	QFont* font = _fontColl->getFont(idFont);
	if (!font) {
		QFont fontApp = QApplication::font();
	}
	return font;

}

bool  uoReportDoc::addFont(QString family)
{
	bool retVal = false;
	if (!family.isEmpty())
	{
		int fontId =  _fontColl->findFont(family);
		if (fontId == -1){
			int fontId = _fontColl->addFont(family);
		}
		retVal = (fontId == -1) ? false : true;
	}
	return retVal;
}


/// Вернуть цвет из коллекции
QColor*  uoReportDoc::getColorByID(const int idColor)
{
	QColor* col = NULL;
	return col;
}


/// создаем новую структуру uoCellTextProps для хранения атрибутов текста.
/// теперь есть возможность вставить акселератор для считывания больших документов.
uoCellTextProps* uoReportDoc::getNewTextProp()
{
	///\todo сбацать акселераторы под считывание документов
	uoCellTextProps* prop = new uoCellTextProps();
	prop->_fontID = 0;
	if (_fontColl->countFonts() == 0)
	{
		QFont fontApp = QApplication::font();
		prop->_fontID = _fontColl->addFont(fontApp.family());
	}

	return prop;
}

/// Создаем новую uoCellJoin
uoCellJoin* uoReportDoc::getCellJoin()
{
	return m_cellJoinCash.getItem();
}

/// Утилизируем uoCellJoin
void uoReportDoc::saveCellJoin(uoCellJoin* cellJItem)
{
	m_cellJoinCash.saveItem(cellJItem);
	cellJItem = NULL;
}


/// создаем новую структуру uoCellTextProps для хранения атрибутов текста.
/// теперь есть возможность вставить акселератор для считывания больших документов.
uoCellBordProps* uoReportDoc::getNewBordProp()
{
	///\todo сбацать акселераторы под считывание документов
	uoCellBordProps* prop = new uoCellBordProps();
	return prop;
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

/// Тестирование класса.
void uoReportDoc::test(){

    int nTestOk = 0, nTestAll = 0;
    bool printAll = true;
	bool printCurent = true;

	qreal sz = 17;

    qDebug()<<"Start test class \"uoReportDocBody\"";
    qDebug()<<"{";
	if (false || printAll) {
		setScaleSize(rhtVertical, 2, 15);
		setScaleSize(rhtVertical, 4, 18); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 3, 19); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 5, 25); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 2, sz); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 1, sz); if (printCurent) _headerV->printToDebug();
		setScaleSize(rhtVertical, 7, 11); if (printCurent) _headerV->printToDebug();

		toDebugTest(getScaleSize(rhtVertical, 1) == sz, &nTestOk, &nTestAll, "getScaleSize(rhtVertical, 1) == sz");
		toDebugTest(getScaleSize(rhtVertical, 2) == sz, &nTestOk, &nTestAll, "getScaleSize(rhtVertical, 2) == sz");
		qDebug() << " size " << _headerV->getCountItem();
		qDebug() << " _headerV->deleteItem(2,2); ";
		_headerV->deleteItem(2,2);
		qDebug() << " size " << _headerV->getCountItem();
		_headerV->printToDebug();
	}


    qDebug()<<"Test all: "<<nTestAll<<" test OK: "<< nTestOk<<" test is: "<< (nTestOk==nTestAll);
    qDebug()<<"End test class \"uoReportDocBody\"";
    qDebug()<<"}";

}

} // namespace uoReport
