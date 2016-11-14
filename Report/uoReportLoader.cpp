/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReport.h"
#include "uoReportLoader.h"
#include "uoReportManager.h"
#include "uoSpanTree.h"
#include "uoReportDoc.h"
#include "uoReportDocBody.h"
#include "uoCellMatrix.h"
#include <QFile>
#include <QObject>
#include <QtXml>
#include <QDomDocument>
#include <QApplication>
#include <QTemporaryFile>
#include <QBuffer>

namespace uoReport {
/**
	\class uoReportLoader - сериализация uoReportDoc
	\brief Класс сериализации документа uoReportDoc. Сохраняет, восстанавливает документ.

		Предполагаю несколько форматов обратимого хранения документа:
		- двоичный формат;
		- XML формат.

		Так-же документ можно будет сохранять в форматах HTML и в дальних планах в форматах
		xls и ОО-Calc. Но это в будующем. На сколько далеком, не могу сказать.

		uoReportLoader - класс "обходчик"
*/

#define UORPT_XMLTYPEDOC 			"uoReportDocXML"
#define UORPT_STRINGNAME_GROUP 		"group"
#define UORPT_STRINGNAME_SECTION 	"section"

class uoReportLoaderXML;
//====================================================================================
//====================================================================================
uoReportLoader::~uoReportLoader(){
}

void uoReportLoader::setFileName(QString fileName)
{
	m_docFilePath = fileName;
}

/// Проверим установлены ли проперти для сохранения/восстановления данных.
bool uoReportLoader::validateStoreProps(bool forLoad){

	if (m_docFilePath.isEmpty() ||
		m_storeFormat == uoRsf_Unknown)
	{
		return false;
	}
	if (forLoad) {
		/// при чтении.
		if (!QFile::exists(m_docFilePath)){
			setLastError(QString(QObject::tr("File '%1' not exist")).arg(m_docFilePath));
			return false;
		}
	} else {
		/// При записи.
	}
	return true;
}

/// Статическая функция, возвращает экземпляр "грузчика" по формату.
uoReportLoader* uoReportLoader::getLoader(uoRptStoreFormat stFormat)
{
	if (stFormat == uoRsf_Unknown ||
		stFormat == uoRsf_Binary ||
		stFormat == uoRsf_HTML) {
			return NULL;
	} else if (stFormat == uoRsf_XML || stFormat == uoRsf_XML_Mime) {
		uoReportLoader* loader = new uoReportLoaderXML();
		loader->setFormat(stFormat);
		return loader;
	} else if (stFormat == uoRsf_TXL) {
		uoReportLoader* loader = new uoReportLoaderTXT();
		loader->setFormat(stFormat);
		return loader;
	}
	return NULL;
}

QMap<uoRptStoreFormat,QString> uoReportLoader::getAviableLoadFormat(QString& filter)
{
	QMap<uoRptStoreFormat,QString> map;
	map.insert(uoRsf_XML, QObject::tr("XML Files (*.xml)"));
	map.insert(uoRsf_TXL, QObject::tr("Text files (*.txt)"));
	filter = "XML Files (*.xml);;Text files (*.txt)";
	return map;

}
QMap<uoRptStoreFormat,QString> uoReportLoader::getAviableStoreFormat(QString& filter)
{
	QMap<uoRptStoreFormat,QString> map;
	map.insert(uoRsf_XML, QObject::tr("XML Files (*.xml)"));
	map.insert(uoRsf_TXL, QObject::tr("Text files (*.txt)"));
	filter = "XML Files (*.xml);;Text files (*.txt)";
	return map;
}

uoRptStoreFormat uoReportLoader::getFormatByName(QString fileName)
{
	uoRptStoreFormat format = uoRsf_Unknown;
	if (!fileName.isEmpty()){
		if (fileName.endsWith(QString(".xml"), Qt::CaseInsensitive)){
			format = uoRsf_XML;
		} else if (fileName.endsWith(QString(".txt"), Qt::CaseInsensitive)){
			format = uoRsf_TXL;
		}
	}
	return format;
}


QString	uoReportLoader::getFileName() {
	if (!m_docFilePath.isEmpty())
		return m_docFilePath;
	QTemporaryFile file;
	if (file.open()) {
		 m_docFilePath = file.fileName();// returns the unique file name
		 qDebug() << "QTemporaryFile.fileName()" << m_docFilePath;
		 file.close();
		 QFile::remove(m_docFilePath);
	}
	return m_docFilePath;
}


//====================================================================================
//====================================================================================
/**
	\class uoReportLoaderXML - сериализация uoReportDoc в XML
	\brief Класс сериализации документа uoReportDoc в XML.
	Структура выгрузки в XML.


*/
uoReportLoaderXML::uoReportLoaderXML(){
	setFormat(uoRsf_XML);
	m_version = uoReportVersion;
	m_storeOper = uorSO_Unknown;
	m_enableProcDialog = true;
}

void uoReportLoaderXML::save(QString str)
{
	if (m_storeOper == uorSO_SaveToFile){
		_textStream << str;
	} else if (m_storeOper == uorSO_SaveToBArray){
		m_byteArray += str;
	}
}

///Инициализация лоадера. Конкренто  QTextStream и QFile
bool uoReportLoaderXML::initFO(bool forLoad){

	m_storeOper = (forLoad) ? uorSO_LoadFromFile : uorSO_SaveToFile;
	if (m_storeFormat == uoRsf_XML) {
		if(!validateStoreProps(forLoad)) {
			return false;
		}
		_outFile.setFileName(getFileName());
		_textStream.setDevice(&_outFile);
	}

	_textStream.setCodec("UTF-8");

	if (m_storeFormat == uoRsf_XML) {
		bool isOpen = false;
		if (forLoad) {
			isOpen = _outFile.open(QIODevice::ReadOnly | QIODevice::Text);
		} else {
			isOpen = _outFile.open(QIODevice::WriteOnly | QIODevice::Text);
		}
		if (!isOpen){
			setLastError(QString("Can not open file: %1").arg(getFileName()));
			return false;
		}
	}

	return true;
}

bool uoReportLoaderXML::initBA(uorStoreOperation sOper, QByteArray& byteArray)
{
	m_storeOper = sOper;
	if (sOper == uorSO_LoadFromBArray)
		m_byteArray = QByteArray(byteArray);
	if (!(sOper == uorSO_LoadFromBArray || sOper == uorSO_SaveToBArray)) {
		qWarning() << "Only uorSO_LoadFromBArray || uorSO_SaveToBArray";
		return false;
	}
	if (m_storeOper == uorSO_SaveToFile){
//		bool forLoad = (sOper == uorSO_LoadFromBArray) ? true : false;
		_outFile.setFileName(getFileName());
		_textStream.setDevice(&_outFile);

		_textStream.setCodec("UTF-8");

		bool isOpen = false;
		isOpen = _outFile.open(QIODevice::ReadWrite | QIODevice::Text);
		if (!isOpen){
			setLastError(QString("Can not open file: %1").arg(getFileName()));
			return false;
		}
	}
	return true;
}

/// Выгрузить все в QByteArray
///\todo - убить
bool uoReportLoaderXML::readAll(QByteArray& baArr)
{
	_textStream >> baArr;
	return true;
}

bool uoReportLoaderXML::saveDocStart(uoReportDoc* doc){

	QString outStr = "";
	m_doc = doc;
	outStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	save(outStr); //m_byteArray.append(outStr);
	outStr = QString("<Doc type = \"%1\" version = \"%2\">\n").arg(UORPT_XMLTYPEDOC).arg(uoReportVersion);
	//_textStream << outStr; m_byteArray.append(outStr);
	save(outStr);

	/*
	_textStream << QString("<!-- Scales and Group types \"%1\"=Vertical  \"%2\"=Horizontal -->\n").arg(uorRhtRowsHeader).arg(uorRhtColumnHeader);
	_textStream << QString(
	"<!-- Cell format descr:  Cell no = \"number cell\" fid = \"font ID\"\n  "
	"fsz = \"font size\" cId = \"color ID\" cbgid = \"BG color ID\" \n"
	"tb = \"Text behavior\" ah = \"Alignment hor.\" av = \"Alignment ver.\" tt= \"Text type\" \n"
	" bt_l = \"left bord type\" bt_t = \"top bord type\" bt_r = \"right bord type\" bt_b = \"bottom bord type\"\n"
	" bt_c = \"border color\" -->\n");
	*/


	outStr = QString("<DocSize>\n");
	save(outStr); //_textStream << outStr; m_byteArray.append(outStr);
	outStr = QString("\t<Sizes row = \"%1\" col = \"%2\" sizeV = \"%3\" sizeH = \"%4\" />\n")
		.arg(doc->getRowCount())
		.arg(doc->getColCount())
		.arg(doc->getVSize())
		.arg(doc->getHSize());
	save(outStr); // _textStream << outStr; m_byteArray.append(outStr);

	outStr = QString("\t<SizesV sizeV = \"%3\" sizeH = \"%4\" />\n")
		.arg(doc->getVSize(true))
		.arg(doc->getHSize(true));
	save(outStr);
	outStr =  QString("\t<DefSizes row = \"%1\" col = \"%2\"/>\n")
		.arg(doc->getDefScaleSize(uorRhtRowsHeader))
		.arg(doc->getDefScaleSize(uorRhtColumnHeader));
	save(outStr);
	save(QString("</DocSize>\n"));


	return true;
}

/// Запись данных хейдера: секции, группировки
bool uoReportLoaderXML::saveGroupsHeaderStart(int count, uoRptHeaderType rht){
	save(QString("<Groups type = \"%1\" count = \"%2\">\n").arg(rht).arg(count));
	return true;
}

bool uoReportLoaderXML::saveGroupsItem(uoLineSpan* peSpn){
	save(QString("\t<Group start = \"%1\" end = \"%2\" folded=\"%3\"/>\n")
	.arg(peSpn->getStart()).arg(peSpn->getEnd()).arg(peSpn->getFolded()));
	return true;
}
bool uoReportLoaderXML::saveGroupsHeaderEnd(uoRptHeaderType rht){
	save(QString("</Groups>\n"));
	return true;
}

bool uoReportLoaderXML::saveSectionHeaderStart(int count, uoRptHeaderType rht){
	QString rhtS = (rht == uorRhtRowsHeader) ? "rows" : "cols";
	save(QString("<Sections type = \"%1\" count = \"%2\" type_s = \"%3\">\n").arg(rht).arg(count).arg(rhtS));
	return true;
}
bool uoReportLoaderXML::saveSectionItem(uoLineSpan* peSpn){
	save(QString("\t<Section start = \"%1\" end = \"%2\" name=\"%3\" level=\"%4\" />\n")
	.arg(peSpn->getStart()).arg(peSpn->getEnd()).arg(peSpn->m_name).arg(peSpn->m_level));
	return true;
}
bool uoReportLoaderXML::saveSectionHeaderEnd(uoRptHeaderType rht){
	save(QString("</Sections>\n"));
	return true;
}

bool uoReportLoaderXML::saveScaleHeaderStart(int count, uoRptHeaderType rht){
	QString rhtS = (rht == uorRhtRowsHeader) ? "rows" : "cols";
	save(QString("<Scales type = \"%1\" count = \"%2\" type_s = \"%3\">\n").arg(rht).arg(count).arg(rhtS));
	return true;
}
bool uoReportLoaderXML::saveScaleItem(uoRptNumLine* rLine){
	save(QString("\t<Scale no = \"%1\" hide = \"%2\" size=\"%3\" fixed=\"%4\"/>\n")
	.arg(rLine->number()).arg(rLine->hiden()).arg(rLine->size()).arg(rLine->fixed()));
	return true;
}
bool uoReportLoaderXML::saveScaleHeaderEnd(uoRptHeaderType rht){
	save(QString("</Scales>\n"));
	return true;
}

void uoReportLoaderXML::saveRowsStart(int rowCount){
	save(QString("<Rows count = \"%1\">\n").arg(rowCount));
}


void uoReportLoaderXML::saveRowItemStart(uoRow* row){
	int rowNumb = row->number();
	int cellCount = row->getCountItem();
	QString outStr;
	outStr = QString("\t<RowItem no = \"%1\" cellCnt = \"%2\" LMaxToRight = \"%3\" LMaxToLeft = \"%4\" LFromCell = \"%5\" lengthMaxOver = \"%6\" >\n")
	.arg(rowNumb)
	.arg(cellCount)
	.arg(row->m_lengthMaxToRight)
	.arg(row->m_lengthMaxToLeft)
	.arg(row->m_lengthFromCell)
	.arg(row->m_lengthMaxOver)
	;
	save(outStr);
}

void uoReportLoaderXML::saveCell(uoCell* cellItem){
	QString text = cellItem->getText();
	text = Qt::escape(text);
	uorBorderPropBase* bp = cellItem->getBorderProp();
	uorTextDecor* td = cellItem->getTextProp(m_doc,false);
	QString strFont;
	QString strTextT;
	QString bpStr;
	QString cellJoinTxt;

	if (td){

		strFont = QString("font=\"%1;%2;%3;%4;%5;%6;%7\" ")
		.arg(cellItem->getFontId())
		.arg(cellItem->getFontSize())
		.arg(cellItem->getFontB())
		.arg(cellItem->getFontI())
		.arg(cellItem->getFontU())
		.arg(cellItem->getFontColorId())
		.arg(cellItem->getBGColorId())
		;

		strTextT = QString("textAtr=\"%1;%2;%3;%4\" ")
		.arg(cellItem->getAlignmentHor())
		.arg(cellItem->getAlignmentVer())
		.arg(cellItem->getTextType())
		.arg(cellItem->getTextBehavior())
		;
	}

	if (bp){
		bpStr = QString(" border=\"%1;%2;%3;%4;%5\" ")
		.arg(bp->m_bordType[0])
		.arg(bp->m_bordType[1])
		.arg(bp->m_bordType[2])
		.arg(bp->m_bordType[3])
		.arg(bp->m_bordColor);
	}
	uoCellJoin* cellJoinItem = 0;
	if (cellItem->isUnionHas()){
		cellJoinItem = cellItem->m_ceelJoin;
		cellJoinTxt = QString(" join=\"%1;%2;%3\" ")
		.arg(cellJoinItem->m_JoinType)
		.arg(cellJoinItem->m_row)
		.arg(cellJoinItem->m_col);
	}




	QString strAllCell = QString("\t\t<Cell no = \"%1\" %2 %3 %4 %5>%6</Cell>\n")
	.arg(cellItem->number())
	.arg(strFont)
	.arg(strTextT)
	.arg(bpStr)
	.arg(cellJoinTxt)
	.arg(text)
	;
	save(strAllCell);


	if(false) {
		// для версии 0.1
		qDebug() << strAllCell;


		QString str = QString("\t\t<Cell no = \"%1\" fid = \"%2\"  fsz = \"%3\" cId = \"%4\" cbgid = \"%5\" tb = \"%6\" ah = \"%7\" av = \"%8\" tt= \"%9\"")
		.arg(cellItem->number())
		.arg(cellItem->getFontId())
		.arg(cellItem->getFontSize())
		.arg(cellItem->getFontColorId())
		.arg(cellItem->getBGColorId())
		.arg(cellItem->getTextBehavior())
		.arg(cellItem->getAlignmentHor())
		.arg(cellItem->getAlignmentVer())
		.arg(cellItem->getTextType())
		;
		if (bp){
			bpStr = QString(" bt_l = \"%1\" bt_t = \"%2\" bt_r = \"%3\" bt_b = \"%4\" bt_c = \"%5\" ")
			.arg(bp->m_bordType[0])
			.arg(bp->m_bordType[1])
			.arg(bp->m_bordType[2])
			.arg(bp->m_bordType[3])
			.arg(bp->m_bordColor);
		}

		QString text2 = QString("%1 %2>%3</Cell>\n").arg(str).arg(bpStr).arg(text);
		save(text2);
	}

}

void uoReportLoaderXML::saveRowItemEnd(){
	save(QString("\t</RowItem>\n"));
	//qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void uoReportLoaderXML::saveRowsEnd(){
	save(QString("\t</Rows>\n"));
}

void uoReportLoaderXML::saveSelectionStart(uorSelectionType selectionType){
	save(QString("\t<Selections type = \"%1\">\n").arg(selectionType));
}

void uoReportLoaderXML::saveSelectionRCStart(uorSelectionType selectionType, int count){
	save(QString("\t\t<SelectionsRC type = \"%1\" count = \"%2\" >\n").arg(selectionType).arg(count));
}
void uoReportLoaderXML::saveSelectionRCItems(QList<int>& int_list){
	if (int_list.isEmpty())
		return;
	QList<int>::const_iterator iter = int_list.constBegin();
	while(iter != int_list.constEnd()){
		save(QString("%1;").arg(*iter));
		++iter;
	}

}
void uoReportLoaderXML::saveSelectionRCEnd(){
	save(QString("\t\t</SelectionsRC>\n"));
}
void uoReportLoaderXML::saveSelectionCellsStart(uorSelectionType selectionType, int count){
	save(QString("\t\t<SelectedCells type = \"%1\" count = \"%2\" >\n").arg(selectionType).arg(count));
}
void uoReportLoaderXML::saveSelectionCell(const QPoint& point, int index, int count){
	save(QString("%1x%2;").arg(point.y()).arg(point.x()));
}
void uoReportLoaderXML::saveSelectionCellsEnd(){
	save(QString("\t\t</SelectedCells>\n"));
}

void uoReportLoaderXML::saveSelectionEnd(){
	save(QString("\t</Selections>\n"));
}



void uoReportLoaderXML::saveMatrixStart(uoCellMatrix* matrx)
{
	save(QString("\t<Matrix row = \"%1\" col = \"%2\" >\n").arg(matrx->rows()).arg(matrx->cols()));
}
void uoReportLoaderXML::saveMatrixRowStart(int row)
{
	save(QString("\t<Matrixrow nom = \"%1\">\n").arg(row));
}

void uoReportLoaderXML::saveMatrixRowEmd(){
	save(QString("\t</Matrixrow>\n"));

}
void uoReportLoaderXML::saveMatrixEnd(uoCellMatrix* matrx)
{
	Q_UNUSED(matrx);
	save(QString("\t</Matrix>\n"));
}



void uoReportLoaderXML::saveFontStart(int count)
{
	save(QString("\t<Fonts count = \"%1\">\n").arg(count));
}

/// сериализуем только имя...
void uoReportLoaderXML::saveFont(QFont* psFont, int nom)
{
	save(QString("\t\t<Font name = \"%1\" />\n").arg(psFont->family()));
}
void uoReportLoaderXML::saveFontEnd()
{
	save(QString("\t</Fonts>\n"));
}

void uoReportLoaderXML::saveArbitraryBlockStart(QString blockName)
{
	save(QString("\t\t<ArbitraryBlock name = \"%1\">\n").arg(blockName));
}
void uoReportLoaderXML::saveArbitraryBlockItem(QString itemString)
{
	save(QString("\t\t<ArbitraryItem> %1 </ArbitraryItem>\n").arg(itemString));
}
void uoReportLoaderXML::saveArbitraryBlockEnd(QString blockName)
{
	save(QString("\t<!-- end of %1 -->\n").arg(blockName));
	save(QString("\t</ArbitraryBlock>\n"));
}


/// Запись подвальной части
bool uoReportLoaderXML::saveDocEnd(uoReportDoc* doc){
	save(QString("</Doc>\n"));

	return true;
}
bool uoReportLoaderXML::finalize(){
	if (m_storeOper == uorSO_SaveToFile) {
		_textStream.flush();
		if (_outFile.isOpen())
			_outFile.close();
		if (m_storeOper == uorSO_SaveToBArray){
			QFile file(getFileName());
			if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
				m_byteArray = file.readAll();
				file.close();
				QFile::remove(getFileName());
			}
		}
	}
	return true;
}

bool uoReportLoaderXML::loadGroupsHeader(const QDomElement &node,uoReportDoc* doc)
{
	bool retVal = true;
	int groupType = node.attribute("type").toInt();
	if (groupType == 1 || groupType == 2)
	{
		uoRptHeaderType rht = (groupType == 1) ? uorRhtRowsHeader : uorRhtColumnHeader;
		QDomElement child = node.firstChildElement();

		int start = 0, stop = 0, folded = 0;
		while (!child.isNull()) {
			start = child.attribute("start").toInt();
			stop =  child.attribute("end").toInt();
			folded = child.attribute("folded").toInt();
			doc->addGroup(start, stop, rht, (bool)folded); ///\todo там надо прятать строки, пока не до...

			child = child.nextSiblingElement();
		}
	} else {
		qWarning() << "Bad attribute for group...";
	}

	return retVal;
}
bool uoReportLoaderXML::loadSectionHeader(const QDomElement &node,uoReportDoc* doc)
{
	bool retVal = true;
	int groupType = node.attribute("type").toInt();
	if (groupType == 1 || groupType == 2)
	{
		uoRptHeaderType rht = (groupType == 1) ? uorRhtRowsHeader : uorRhtColumnHeader;
		QDomElement child = node.firstChildElement();
		QString nameSect;
		int start = 0, stop = 0;
		while (!child.isNull()) {
			start = child.attribute("start").toInt();
			stop =  child.attribute("end").toInt();
			nameSect = child.attribute("name");
			doc->addSection(start, stop, rht, nameSect); // а тут надо гарантировать, что имя уникальное...

			child = child.nextSiblingElement();
		}
	} else {
		qWarning() << "Bad attribute for Section...";
	}

	return retVal;
}


bool uoReportLoaderXML::loadScalesHeader(const QDomElement &node, uoReportDoc* doc)
{
	bool retVal = true;
	int groupType = node.attribute("type").toInt();
	if (groupType == 1 || groupType == 2)
	{
		uoRptHeaderType rht = (groupType == 1) ? uorRhtRowsHeader : uorRhtColumnHeader;
		QDomElement child = node.firstChildElement();
		QString nameSect;
		int number = 0;
		bool hide = false, fixed = false;
		uorNumber size = uorNumberNull;
		while (!child.isNull()) {
			number = child.attribute("no").toInt();
			hide =  child.attribute("hide").toInt();
			size =  (uorNumber)child.attribute("size").toFloat();
			fixed = child.attribute("fixed").toInt();
			doc->setScaleSize(rht, number, size);
			if (hide)
				doc->setScalesHide(rht, number, hide);
			if (fixed)
				doc->setScaleFixedProp(rht, number, fixed);

			child = child.nextSiblingElement();
		}
	} else {
		qWarning() << "Bad attribute for Section...";
	}

	return retVal;
}


uoCell* uoReportLoaderXML::loadCell(const QDomElement &node, uoReportDoc* doc, int rowNum)
{
	/*	исправление бага, почемуто накапливаются символы '0D' '0D' '0D'
		когда сохраняю текст, содержащий возврат коретки	*/
	static const QString ch = "\n";
	static const QString ch2 = "\r\n";
	uoCell* cell = 0;
	int cellNumber;
	int other = 0;
	QString cellText;
	cellNumber = node.attribute("no").toInt();
	cellText = node.text();

	/*	исправление бага, почемуто накапливаются символы '0D' '0D' '0D'
		когда сохраняю текст, содержащий возврат коретки	*/
	while(cellText.indexOf(ch2)>=0)
		cellText = cellText.replace(ch2, ch);

	doc->setCellText(rowNum,cellNumber,cellText);
	cell = doc->getCell(rowNum,cellNumber,true,true);
	if (cell){
		if (m_version == "0.1"){
			other = node.attribute("fid").toInt();			cell->m_textProp->m_fontId = other;
			other = node.attribute("fsz").toInt();			cell->m_textProp->m_fontSz = other;
			other = node.attribute("tb").toInt();			cell->m_textProp->m_TextBehavior = (uoCellTextBehavior)other;
			other = node.attribute("ah").toInt();			cell->m_textProp->m_horTAlignment = (uoHorAlignment)other;
			other = node.attribute("av").toInt();			cell->m_textProp->m_vertTAlignment = (uoVertAlignment)other;
			other = node.attribute("tt").toInt();			cell->m_textProp->m_textType = (uoCellTextType)other;
			other = node.attribute("bt_l").toInt();		cell->m_borderProp->m_bordType[0] = (uoCellBorderType)other;
			other = node.attribute("bt_t").toInt();		cell->m_borderProp->m_bordType[1] = (uoCellBorderType)other;
			other = node.attribute("bt_r").toInt();		cell->m_borderProp->m_bordType[2] = (uoCellBorderType)other;
			other = node.attribute("bt_b").toInt();		cell->m_borderProp->m_bordType[3] = (uoCellBorderType)other;
		} else
		if(m_version == "0.2" || m_version == "0.3")
		{
			QChar separator = ';';
			if (m_version == "0.2")
				separator = '#';
			QString atrVal, str;
			QStringList sList;
			str = node.attribute("font");
			if (!str.isEmpty()){
				sList = str.split(separator);
				if (sList.size() == 7){
					other = sList.at(0).toInt();	cell->m_textProp->m_fontId = other;
					other = sList.at(1).toInt();	cell->m_textProp->m_fontSz = other;
					other = sList.at(2).toInt();	cell->m_textProp->m_fontB = other;
					other = sList.at(3).toInt();	cell->m_textProp->m_fontI = other;
					other = sList.at(4).toInt();	cell->m_textProp->m_fontU = other;
					///\todo тут цвета, доработать
//						other = sList.at(5).toInt();	cell->m_textProp->m_fontU = other;
//						other = sList.at(6).toInt();	cell->m_textProp->m_fontU = other;

				}
			}
			str = node.attribute("textAtr");
			if (!str.isEmpty()){
				sList = str.split(separator);
				if (sList.size() == 4){
					other = sList.at(0).toInt();	cell->m_textProp->m_horTAlignment = (uoHorAlignment)other;
					other = sList.at(1).toInt();	cell->m_textProp->m_vertTAlignment = (uoVertAlignment)other;
					other = sList.at(2).toInt();	cell->m_textProp->m_textType = (uoCellTextType)other;
					other = sList.at(3).toInt();	cell->m_textProp->m_TextBehavior = (uoCellTextBehavior)other;
				}
			}
			str = node.attribute("border");
			if (!str.isEmpty()){
				sList = str.split(separator);
				if (sList.size() == 5){
					other = sList.at(0).toInt();	cell->m_borderProp->m_bordType[0] = (uoCellBorderType)other;
					other = sList.at(1).toInt();	cell->m_borderProp->m_bordType[1] = (uoCellBorderType)other;
					other = sList.at(2).toInt();	cell->m_borderProp->m_bordType[2] = (uoCellBorderType)other;
					other = sList.at(3).toInt();	cell->m_borderProp->m_bordType[3] = (uoCellBorderType)other;
					///\todo тут цвет.
//						other = sList.at(4).toInt();
				}
			}
			str = node.attribute("join");
			if (!str.isEmpty()){
				sList = str.split(separator);
				if (sList.size() == 3){
					if (cell->provideJoinProp(doc)){
						other = sList.at(0).toInt();	cell->m_ceelJoin->m_JoinType = (uoCellsJoinType)other;
						other = sList.at(1).toInt();	cell->m_ceelJoin->m_row = other;
						other = sList.at(2).toInt();	cell->m_ceelJoin->m_col = other;
					}
					///\todo тут цвет.
//						other = sList.at(4).toInt();
				}
			}
		}
	}
	return cell;
}


bool uoReportLoaderXML::loadCellsByRow(const QDomElement &node, uoReportDoc* doc, int rowNum)
{
	bool retVal = true;
	QDomElement child = node.firstChildElement();

	while (!child.isNull()) {
		loadCell(child, doc, rowNum);
		child = child.nextSiblingElement();
	}
	return retVal;
}

/**
	Окончание процесса обработки строки, для обновление прогресс-бара.

*/
bool uoReportLoaderXML::endProcessRow(int row)
{
	bool retVal = false;
	if (m_enableProcDialog) {
		if (m_progresDlg) {
			m_progresDlg->setValue(row);
			m_progresDlg->setLabelText(QObject::tr("Load %1 -> %2").arg(getFileName()).arg(row));
			retVal = m_progresDlg->wasCanceled();
		}
	}
	return retVal;
}

bool uoReportLoaderXML::loadRows(const QDomElement &node, uoReportDoc* doc)
{
	bool retVal = true;
	QDomElement child = node.firstChildElement();
	int rowNumber;
	while (!child.isNull()) {
		rowNumber = child.attribute("no").toInt();
		loadCellsByRow(child, doc, rowNumber);
		child = child.nextSiblingElement();
		if (endProcessRow(rowNumber))
			break;
	}

	return retVal;
}


bool uoReportLoaderXML::loadFont(const QDomElement &node, uoReportDoc* doc)
{
	bool retVal = true;
	QDomElement child = node.firstChildElement();
	int fontId = 0;
	doc->clearFonts();
	QString fontName;
	while (!child.isNull()) {
		fontName = child.attribute("name");
		fontId = doc->addFont(fontName);
		child = child.nextSiblingElement();
	}

	return retVal;

}

bool uoReportLoaderXML::loadSelections(const QDomElement &node, uoReportDoc* doc)
{
	bool retVal = true;
	uoReportSelection* 	sel = doc->selection();
	if (sel) {
		sel->clearSelections();
		uorSelectionType sMode = sel->convertIntToSelMode(node.attribute("type").toInt());
		sel->setSelectionMode(sMode);
		QDomElement child = node.firstChildElement();
		QString nodeName, linePart;
		QStringList strList, strList2;
		QPoint pt;

		while (!child.isNull()) {
			nodeName = child.nodeName();
//			int cnt = child.attribute("count").toInt();
			QString nodeSrc = child.text();
			bool isCellProcc = false;
			if (nodeName.compare("SelectedCells",Qt::CaseInsensitive) == 0){
				isCellProcc = true;
			}
			strList = nodeSrc.split(";");
			int row = 0, col = 0, rowOrCol = 0;

			for (int y=1; y<=strList.size(); y++){
				linePart = strList.at(y-1);
				linePart = linePart.trimmed();
				if (!linePart.isEmpty()){
					if (isCellProcc){
						strList2 = linePart.split("x");
						row = strList2.at(0).toInt();
						col = strList2.at(1).toInt();
						if (row > 0 && col > 0){
							sel->selectCell(col,row, false);
						}
					} else {
						rowOrCol = linePart.toInt();
						if (rowOrCol > 0){
							if (sMode == uoRst_Row || sMode == uoRst_Rows){
								sel->selectRow(rowOrCol, false);
							} else {
								sel->selectCol(rowOrCol, false);
							}
						}
					}
				}
			}
			child = child.nextSiblingElement();
		}
	}
	return retVal;
}

bool uoReportLoaderXML::loadMatrix(const QDomElement &node, uoReportDoc* doc)
{
	bool retVal = true;
	uoCellMatrix* matrix = 0; //	doc->getCellMatrix();
	int matr_row = 0, matr_col = 0;
	matr_row = node.attribute("row").toInt();
	matr_col = node.attribute("col").toInt();
	if ((matr_row * matr_col) == 0){
		qWarning() << "broken matrix size";
		return false;
	}
	matrix = new uoCellMatrix(matr_row, matr_col);
	if(matrix){
		uoCell* cell = 0;
		QDomElement childRow = node.firstChildElement();
		QDomElement childCell;
		int rowNumber;
		while (!childRow.isNull()) {
			rowNumber = childRow.attribute("nom").toInt();

			childCell = childRow.firstChildElement();
			while (!childCell.isNull()) {
				cell = loadCell(childCell, doc, rowNumber);
				if (cell){
					matrix->setCell(rowNumber, cell->number(), cell);
				}
				childCell = childCell.nextSiblingElement();
			}
			childRow = childRow.nextSiblingElement();
			if (endProcessRow(rowNumber))
				break;
		}
	}
	doc->setCellMatrix(matrix);
	return retVal;
}

QString uoReportLoaderXML::readAll()
{
	return _textStream.readAll();
}

bool uoReportLoaderXML::load(uoReportDoc* doc)
{
	bool retVal = true;
	QDomDocument xmldoc;
	if (m_enableProcDialog) {
		m_progresDlg = uoReportManager::instance()->progressDlg();
		if (!m_progresDlg){
			qWarning()<<"Error create progressDlg";
			return false;
		}
		m_progresDlg->setLabelText(QObject::tr("Load %1").arg(getFileName()));
		m_progresDlg->setModal(true);
		m_progresDlg->show();
	}
	if (m_storeOper == uorSO_LoadFromBArray) {
		xmldoc.setContent(m_byteArray);
	} else {
		xmldoc.setContent(&_outFile);
	}
	QString nodeName;
	m_doc = doc;

	QDomElement docElem = xmldoc.documentElement();
	m_version = docElem.attribute("version");

	if (m_enableProcDialog){
		if (m_progresDlg){
			m_progresDlg->setLabelText(QObject::tr("Load %1").arg(getFileName()));
			m_progresDlg->setRange(0,doc->getRowCount());
			m_progresDlg->setModal(true);
		}
	}

	qDebug() << "Load m_version" << m_version;
	QDomElement child = docElem.firstChildElement();
	while (!child.isNull()) {

		nodeName = child.nodeName();
		if (nodeName.compare("DocSize",Qt::CaseInsensitive) == 0){
			QDomElement child0 = child.firstChildElement();
			if 	(!child0.isNull()) {
				int rowCnt = child.attribute("row").toInt();
				if (m_enableProcDialog) {
				if (rowCnt > 0 && m_progresDlg)
					m_progresDlg->setRange(0,rowCnt);
				}
			}


		} else if (nodeName.compare("Groups",Qt::CaseInsensitive) == 0){
			loadGroupsHeader(child,doc);
		} else if (nodeName.compare("Sections",Qt::CaseInsensitive) == 0){
			loadSectionHeader(child,doc);
		} else if (nodeName.compare("Scales",Qt::CaseInsensitive) == 0){
			loadScalesHeader(child,doc);
		} else if (nodeName.compare("Rows",Qt::CaseInsensitive) == 0){
			loadRows(child,doc);
		} else if (nodeName.compare("Fonts",Qt::CaseInsensitive) == 0){
			loadFont(child,doc);
		} else if (nodeName.compare("Selections",Qt::CaseInsensitive) == 0){
			loadSelections(child,doc);
		} else if (nodeName.compare("Matrix",Qt::CaseInsensitive) == 0){
			///\Todo - сделать в лоадере считывание матриц....
			loadMatrix(child,doc);
		}

		child = child.nextSiblingElement();
	}
	if (m_enableProcDialog) {
		if (m_progresDlg){
			m_progresDlg->hide();
		}
	}
	m_doc = NULL;

	return retVal;
}

//====================================================================================
//====================================================================================
/**
	\class uoReportLoaderTXT - сериализация uoReportDoc в TXT
	\brief Класс сериализации документа uoReportDoc в TXT.
	Структура выгрузки в TXT.


*/
uoReportLoaderTXT::uoReportLoaderTXT(){
	setFormat(uoRsf_TXL);
	m_version = uoReportVersion;
	m_storeOper = uorSO_Unknown;
	m_allTextTab = "";
	m_rowNumber = 0;
	m_colNumber = 0;
}

///Инициализация лоадера. Конкренто  QTextStream и QFile
bool uoReportLoaderTXT::initFO(bool forLoad){
	if(!validateStoreProps(forLoad)) {
		return false;
	}
	_outFile.setFileName(getFileName());
	_textStream.setDevice(&_outFile);

	_textStream.setCodec("UTF-8");

	bool isOpen = false;
	if (forLoad) {
		isOpen = _outFile.open(QIODevice::ReadOnly | QIODevice::Text);
	} else {
		isOpen = _outFile.open(QIODevice::WriteOnly | QIODevice::Text);
	}
	if (!isOpen){
		setLastError(QString("Can not open file: %1").arg(getFileName()));
		return false;
	}

	return true;
}

bool uoReportLoaderTXT::initBA(uorStoreOperation sOper, QByteArray& byteArray)
{
	m_storeOper = sOper;
	if (!(sOper == uorSO_LoadFromBArray || sOper == uorSO_SaveToBArray)) {
		qWarning() << "Only uorSO_LoadFromBArray || uorSO_SaveToBArray";
		return false;
	}

//	bool forLoad = (sOper == uorSO_LoadFromBArray) ? true : false;
	_outFile.setFileName(getFileName());
	_textStream.setDevice(&_outFile);

	_textStream.setCodec("UTF-8");

	bool isOpen = false;
	isOpen = _outFile.open(QIODevice::ReadWrite | QIODevice::Text);
	if (!isOpen){
		setLastError(QString("Can not open file: %1").arg(getFileName()));
		return false;
	}
	return true;
}


bool uoReportLoaderTXT::saveDocStart(uoReportDoc* doc){
	return true;
}

/// Запись данных хейдера: секции, группировки
bool uoReportLoaderTXT::saveGroupsHeaderStart(int count, uoRptHeaderType rht){
	return true;
}

bool uoReportLoaderTXT::saveGroupsItem(uoLineSpan* peSpn){
	return true;
}
bool uoReportLoaderTXT::saveGroupsHeaderEnd(uoRptHeaderType rht){
	return true;
}

bool uoReportLoaderTXT::saveSectionHeaderStart(int count, uoRptHeaderType rht){
	return true;
}
bool uoReportLoaderTXT::saveSectionItem(uoLineSpan* peSpn){
	return true;
}
bool uoReportLoaderTXT::saveSectionHeaderEnd(uoRptHeaderType rht){
	return true;
}

bool uoReportLoaderTXT::saveScaleHeaderStart(int count, uoRptHeaderType rht){
	return true;
}
bool uoReportLoaderTXT::saveScaleItem(uoRptNumLine* rLine){
	return true;
}
bool uoReportLoaderTXT::saveScaleHeaderEnd(uoRptHeaderType rht){
	return true;
}

void uoReportLoaderTXT::saveRowsStart(int rowCount){
}


void uoReportLoaderTXT::saveRowItemStart(uoRow* row){
	m_rowNumber = row->number();
	m_lastcolNumber = row->getMaxNo();
	m_colNumber = 0;
}

void uoReportLoaderTXT::saveMatrixRowStart(int row)
{
	m_colNumber = 0;
	m_rowNumber = row;
}
void uoReportLoaderTXT::saveMatrixRowEmd(){
	m_rowNumber = -1;
	m_colNumber = -1;
	_textStream << QString("\n");
}


void uoReportLoaderTXT::saveCell(uoCell* cellItem){
	int colNumber = cellItem->number();
	while((colNumber - m_colNumber)>1){
		_textStream << "\t";
		m_colNumber += 1;
	}
	m_colNumber = colNumber;
	QString text = cellItem->getText();
	text = Qt::escape(text);
	text.replace(QString("\t"),QString(" "));
	text.replace(QString("\n"),QString(" "));
	text.replace(QString("\r"),QString(" "));
	_textStream << text;
	if (m_lastcolNumber>m_colNumber)
		_textStream << "\t";

}

void uoReportLoaderTXT::saveRowItemEnd(){
	m_rowNumber = -1;
	m_colNumber = -1;
	_textStream << QString("\n");
}


void uoReportLoaderTXT::saveRowsEnd(){}
void uoReportLoaderTXT::saveFontStart(int count){}
/// сериализуем только имя...
void uoReportLoaderTXT::saveFont(QFont* psFont, int nom){}
void uoReportLoaderTXT::saveFontEnd(){}

/// Запись подвальной части
bool uoReportLoaderTXT::saveDocEnd(uoReportDoc* doc){	return true;}

bool uoReportLoaderTXT::finalize(){
	if (_outFile.isOpen())
		_outFile.close();
	if (m_storeOper == uorSO_SaveToBArray){
		QFile file(getFileName());
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
			m_byteArray = file.readAll();
			file.close();
			QFile::remove(getFileName());
		}
	}

	return true;
}


bool uoReportLoaderTXT::load(uoReportDoc* doc)
{
	bool retVal = true;

	QProgressDialog* progresDlg = uoReportManager::instance()->progressDlg();
	if (!progresDlg){
		qWarning() << "Error!!!";
		return false;
	}
	QString docFilePath = getFileName();

	QFile file(docFilePath);
	long sz = file.size();
	long sz_add = 0;

	progresDlg->setLabelText(QObject::tr("Load %1").arg(docFilePath));
	progresDlg->setRange(0,sz);
	progresDlg->setModal(true);

	if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QTextStream qts_in(&file);
		QChar ch = '\t';
		QStringList list;
		QString line, linePart;
		int row = 1;
		while (!qts_in.atEnd()) {
			QString line = qts_in.readLine();
			sz_add += line.length();
			list = line.split(ch);
			for (int y=1; y<=list.size(); y++){
				linePart = list.at(y-1);
				doc->setCellText(row,y,linePart);

				doc->setCellTextAlignment(row,y,uoVA_Center, uoHA_Left, uoCTB_Transfer);
			}
			progresDlg->setValue(sz_add);
			++row;
		}
	}
	progresDlg->hide();
	return retVal;
}

} //namespace uoReport
