/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "QDebug"
#include "uoReportDoc.h"
#include "uoReportLoader.h"

namespace uoReport {


uoReportDoc::uoReportDoc()
	:uoReportDocBody()
	, _spanTreeGrH(new uoSpanTree)
	, _spanTreeGrV(new uoSpanTree)
	, _spanTreeSctH(new uoSpanTree)
	, _spanTreeSctV(new uoSpanTree)

{
	_maxLevelSpanFoldingH = _maxLevelSpanSectionsH = 0;   /// Группировки
	_spanTreeSctH->setCanOnlyOne(true);
	_spanTreeSctV->setCanOnlyOne(true);
	_storeFormat = uoRsf_Unknown;
}

uoReportDoc::~uoReportDoc()
{
	clear();
	delete _spanTreeGrH;
	delete _spanTreeGrV;
	delete _spanTreeSctH;
	delete _spanTreeSctV;
}

/// Очистка секций.
void uoReportDoc::clear()
{
	_spanTreeGrH->clear();
	_spanTreeGrV->clear();
	_spanTreeSctH->clear();
	_spanTreeSctV->clear();
}

/// проверка возможности сформировать/вставить группу в документ.
/// может потребоваться для того, что-бы задизаблить пункты динамического меню
bool uoReportDoc::possiblyAddGroup(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtVertical)	return _spanTreeGrV->possiblyAddSpan(start, end);
	else					return _spanTreeGrH->possiblyAddSpan(start, end);
}

/// добавление группировки в отчет.
bool uoReportDoc::addGroup(int start, int end, uoRptHeaderType ht)
{
	bool retVal = false;
	uoSpanTree* treeGrp = NULL;

	if(ht == rhtVertical)
		treeGrp = _spanTreeGrV;
	else
		treeGrp = _spanTreeGrH;
	retVal = treeGrp->addSpan(start, end);

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

	QList<int>* lineList= treeGrp->onGroupFold(idGrop, fold);
	if (!lineList)
		return;
	if (!lineList->isEmpty()){
		int lineNo = 0;
		for (int i = 0; i<lineList->size(); i++){
			lineNo = lineList->at(i);
			uoReportDocBody::setScalesHide(rht, lineNo, 1, fold);
		}
	}
	delete lineList;
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
bool uoReportDoc::addSection(int start, int end, uoRptHeaderType ht)
{
	if(ht == rhtHorizontal)	return _spanTreeSctH->addSpan(start, end);
	else					return _spanTreeSctV->addSpan(start, end);
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

	uorHeaderScale* headerScale = NULL;

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

} // namespace uoReport
