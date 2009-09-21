/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReport.h"
#include "uoReportLoader.h"
#include "uoSpanTree.h"
#include "uoReportDoc.h"
#include "uoReportDocBody.h"
#include <QFile>
#include <QtXml>
#include <QDomDocument>


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
	} else if (stFormat == uoRsf_XML) {
		return new uoReportLoaderXML();
	}
	return NULL;
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
}

///Инициализация лоадера. Конкренто  QTextStream и QFile
bool uoReportLoaderXML::init(bool forLoad){
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


bool uoReportLoaderXML::saveDocStart(uoReportDoc* doc){

	QString outStr = "";
	m_doc = doc;
	outStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	_textStream << outStr;
	_textStream << QString("<Doc type = \"%1\" version = \"%2\">\n").arg(UORPT_XMLTYPEDOC).arg(uoReportVersion);
	_textStream << QString("<!-- Scales and Group types \"%1\"=Vertical  \"%2\"=Horizontal -->\n").arg(uorRhtRowsHeader).arg(uorRhtColumnHeader);
	_textStream << QString(
	"<!-- Cell format descr:  Cell no = \"number cell\" fid = \"font ID\"\n  "
	"fsz = \"font size\" cId = \"color ID\" cbgid = \"BG color ID\" \n"
	"tb = \"Text behavior\" ah = \"Alignment hor.\" av = \"Alignment ver.\" tt= \"Text type\" \n"
	" bt_l = \"left bord type\" bt_t = \"top bord type\" bt_r = \"right bord type\" bt_b = \"bottom bord type\"\n"
	" bt_c = \"border color\" -->\n");



	_textStream << QString("<DocSize>\n");
	_textStream << QString("\t<Sizes row = \"%1\" col = \"%2\" sizeV = \"%3\" sizeH = \"%4\" />\n")
		.arg(doc->getRowCount())
		.arg(doc->getColCount())
		.arg(doc->getVSize())
		.arg(doc->getHSize());
	_textStream << QString("\t<SizesV sizeV = \"%3\" sizeH = \"%4\" />\n")
		.arg(doc->getVSize(true))
		.arg(doc->getHSize(true));
	_textStream << QString("\t<DefSizes row = \"%1\" col = \"%2\"/>\n")
		.arg(doc->getDefScaleSize(uorRhtRowsHeader))
		.arg(doc->getDefScaleSize(uorRhtColumnHeader));
	_textStream << QString("</DocSize>\n");


	return true;
}

/// Запись данных хейдера: секции, группировки
bool uoReportLoaderXML::saveGroupsHeaderStart(int count, uoRptHeaderType rht){
	_textStream << QString("<Groups type = \"%1\" count = \"%2\">\n").arg(rht).arg(count);
	return true;
}

bool uoReportLoaderXML::saveGroupsItem(uoLineSpan* peSpn){
	_textStream << QString("\t<Group start = \"%1\" end = \"%2\" folded=\"%3\"/>\n")
	.arg(peSpn->getStart()).arg(peSpn->getEnd()).arg(peSpn->getFolded());
	return true;
}
bool uoReportLoaderXML::saveGroupsHeaderEnd(uoRptHeaderType rht){
	_textStream << QString("</Groups>\n");
	return true;
}

bool uoReportLoaderXML::saveSectionHeaderStart(int count, uoRptHeaderType rht){
	_textStream << QString("<Sections type = \"%1\" count = \"%2\">\n").arg(rht).arg(count);
	return true;
}
bool uoReportLoaderXML::saveSectionItem(uoLineSpan* peSpn){
	_textStream << QString("\t<Section start = \"%1\" end = \"%2\" name=\"%3\" level=\"%4\" />\n")
	.arg(peSpn->getStart()).arg(peSpn->getEnd()).arg(peSpn->_name).arg(peSpn->m_level);
	return true;
}
bool uoReportLoaderXML::saveSectionHeaderEnd(uoRptHeaderType rht){
	_textStream << QString("</Sections>\n");
	return true;
}

bool uoReportLoaderXML::saveScaleHeaderStart(int count, uoRptHeaderType rht){
	_textStream << QString("<Scales type = \"%1\" count = \"%2\">\n").arg(rht).arg(count);
	return true;
}
bool uoReportLoaderXML::saveScaleItem(uoRptNumLine* rLine){
	_textStream << QString("\t<Scale no = \"%1\" hide = \"%2\" size=\"%3\" fixed=\"%4\"/>\n")
	.arg(rLine->number()).arg(rLine->hiden()).arg(rLine->size()).arg(rLine->fixed());
	return true;
}
bool uoReportLoaderXML::saveScaleHeaderEnd(uoRptHeaderType rht){
	_textStream << QString("</Scales>\n");
	return true;
}

void uoReportLoaderXML::saveRowsStart(int rowCount){
	_textStream << QString("<Rows count = \"%1\">\n").arg(rowCount);
}


void uoReportLoaderXML::saveRowItemStart(uoRow* row){
	int rowNumb = row->number();
	int cellCount = row->getCountItem();
	_textStream << QString("\t<RowItem no = \"%1\" cellCnt = \"%2\" LMaxToRight = \"%3\" LMaxToLeft = \"%4\" LFromCell = \"%5\"  >\n")
	.arg(rowNumb)
	.arg(cellCount)
	.arg(row->m_lengthMaxToRight)
	.arg(row->m_lengthMaxToLeft)
	.arg(row->m_lengthFromCell)
	;
}

void uoReportLoaderXML::saveCell(uoCell* cellItem){
	QString text = cellItem->getText();
	text = Qt::escape(text);
	uorBorderPropBase* bp = cellItem->getBorderProp();
	uorTextDecor* td = cellItem->getTextProp(m_doc,false);
	QString strFont;
	QString strTextT;
	QString bpStr;

	if (td){
		strFont = QString("font = \"%1#%2#%3#%4#%5#%6#%7\" ")
		.arg(cellItem->getFontId())
		.arg(cellItem->getFontSize())
		.arg(cellItem->getFontB())
		.arg(cellItem->getFontI())
		.arg(cellItem->getFontU())
		.arg(cellItem->getFontColorId())
		.arg(cellItem->getBGColorId())
		;

		strTextT = QString("textAtr = \"%1#%2#%3#%4\" ")
		.arg(cellItem->getAlignmentHor())
		.arg(cellItem->getAlignmentVer())
		.arg(cellItem->getTextType())
		.arg(cellItem->getTextBehavior())
		;
	}

	if (bp){
		bpStr = QString(" border = \"%1#%2#%3#%4#%5\" ")
		.arg(bp->m_bordType[0])
		.arg(bp->m_bordType[1])
		.arg(bp->m_bordType[2])
		.arg(bp->m_bordType[3])
		.arg(bp->m_bordColor);
	}



	QString strAllCell = QString("\t\t<Cell no = \"%1\" %2 %3 %4>%5</Cell>\n")
	.arg(cellItem->number())
	.arg(strFont)
	.arg(strTextT)
	.arg(bpStr)
	.arg(text)
	;
	_textStream << strAllCell;


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
		_textStream << text2;
	}

}

void uoReportLoaderXML::saveRowItemEnd(){
	_textStream << QString("\t</RowItem>\n");
}

void uoReportLoaderXML::saveRowsEnd(){
	_textStream << QString("\t</Rows>\n");
}


void uoReportLoaderXML::saveFontStart(int count)
{
	_textStream << QString("\t<Fonts count = \"%1\">\n").arg(count);
}

/// сериализуем только имя...
void uoReportLoaderXML::saveFont(QFont* psFont, int nom)
{
	_textStream << QString("\t\t<Font name = \"%1\" />\n").arg(psFont->family());
}
void uoReportLoaderXML::saveFontEnd()
{
	_textStream << QString("\t</Fonts>\n");
}

/// Запись подвальной части
bool uoReportLoaderXML::saveDocEnd(uoReportDoc* doc){
	_textStream << QString("</Doc>\n");

	return true;
}
bool uoReportLoaderXML::finalize(){
	if (_outFile.isOpen())
		_outFile.close();
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
		qreal size = 0.0;
		while (!child.isNull()) {
			number = child.attribute("no").toInt();
			hide =  child.attribute("hide").toInt();
			size =  child.attribute("size").toFloat();
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
bool uoReportLoaderXML::loadCell(const QDomElement &node, uoReportDoc* doc, int rowNum)
{
	bool retVal = true;
	QDomElement child = node.firstChildElement();

	/*	исправление бага, почемуто накапливаются символы '0D' '0D' '0D'
		когда сохраняю текст, содержащий возврат коретки	*/
	static const QString ch = "\n";
	static const QString ch2 = "\r\n";
	uoCell* cell = NULL;
	int cellNumber;
	int other = 0;
	QString cellText;
	while (!child.isNull()) {
		cellNumber = child.attribute("no").toInt();
		cellText = child.text();

		/*	исправление бага, почемуто накапливаются символы '0D' '0D' '0D'
			когда сохраняю текст, содержащий возврат коретки	*/
		while(cellText.indexOf(ch2)>=0)
			cellText = cellText.replace(ch2, ch);

//		qDebug() << "load<<" << cellText;
		doc->setCellText(rowNum,cellNumber,cellText);
		cell = doc->getCell(rowNum,cellNumber,true,true);
		if (cell){
			/* <Cell no="2" fid="0" fsz="8" cId="-1" cbgid="-1" tb="1" ah="1" av="1"
			tt="1" bt_l="1" bt_t="1" bt_r="1" bt_b="1" bt_c="-1"/>

			Cell format descr:  Cell no = "number cell" fid = "font ID"
			fsz = "font size" cId = "color ID" cbgid = "BG color ID"
			tb = "Text behavior" ah = "Alignment hor." av = "Alignment ver." tt= "Text type"
			bt_l = "left bord type" bt_t = "top bord type" bt_r = "right bord type" bt_b = "bottom bord type"
			bt_c = "border color"
			*/
			if (m_version == "0.1"){
				other = child.attribute("fid").toInt();			cell->m_textProp->m_fontId = other;
				other = child.attribute("fsz").toInt();			cell->m_textProp->m_fontSz = other;
				other = child.attribute("tb").toInt();			cell->m_textProp->m_TextBehavior = (uoCellTextBehavior)other;
				other = child.attribute("ah").toInt();			cell->m_textProp->m_horTAlignment = (uoHorAlignment)other;
				other = child.attribute("av").toInt();			cell->m_textProp->m_vertTAlignment = (uoVertAlignment)other;
				other = child.attribute("tt").toInt();			cell->m_textProp->m_textType = (uoCellTextType)other;
				other = child.attribute("bt_l").toInt();		cell->m_borderProp->m_bordType[0] = (uoCellBorderType)other;
				other = child.attribute("bt_t").toInt();		cell->m_borderProp->m_bordType[1] = (uoCellBorderType)other;
				other = child.attribute("bt_r").toInt();		cell->m_borderProp->m_bordType[2] = (uoCellBorderType)other;
				other = child.attribute("bt_b").toInt();		cell->m_borderProp->m_bordType[3] = (uoCellBorderType)other;
			} else
			if(m_version == "0.2")
			{
				QString atrVal, str;
				QStringList sList;
				str = child.attribute("font");
				if (!str.isEmpty()){
					sList = str.split("#");
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
				str = child.attribute("textAtr");
				if (!str.isEmpty()){
					sList = str.split("#");
					if (sList.size() == 4){
						other = sList.at(0).toInt();	cell->m_textProp->m_horTAlignment = (uoHorAlignment)other;
						other = sList.at(1).toInt();	cell->m_textProp->m_vertTAlignment = (uoVertAlignment)other;
						other = sList.at(2).toInt();	cell->m_textProp->m_textType = (uoCellTextType)other;
						other = sList.at(3).toInt();	cell->m_textProp->m_TextBehavior = (uoCellTextBehavior)other;
					}
				}
				str = child.attribute("border");
				if (!str.isEmpty()){
					sList = str.split("#");
					if (sList.size() == 5){
						other = sList.at(0).toInt();	cell->m_borderProp->m_bordType[0] = (uoCellBorderType)other;
						other = sList.at(1).toInt();	cell->m_borderProp->m_bordType[1] = (uoCellBorderType)other;
						other = sList.at(2).toInt();	cell->m_borderProp->m_bordType[2] = (uoCellBorderType)other;
						other = sList.at(3).toInt();	cell->m_borderProp->m_bordType[3] = (uoCellBorderType)other;
						///\todo тут цвет.
//						other = sList.at(4).toInt();
					}
				}
			}


		}

		child = child.nextSiblingElement();
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
		loadCell(child, doc, rowNumber);
		child = child.nextSiblingElement();
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


bool uoReportLoaderXML::load(uoReportDoc* doc)
{
	bool retVal = true;
	QDomDocument xmldoc;
	xmldoc.setContent(&_outFile);
	QString nodeName;
	m_doc = doc;

	QDomElement docElem = xmldoc.documentElement();
	m_version = docElem.attribute("version");
	qDebug() << "Load m_version" << m_version;
	QDomElement child = docElem.firstChildElement();
	while (!child.isNull()) {
//		qDebug() << child.nodeName();
		nodeName = child.nodeName();
		if (nodeName.compare("Groups",Qt::CaseInsensitive) == 0){
			loadGroupsHeader(child,doc);
		} else if (nodeName.compare("Sections",Qt::CaseInsensitive) == 0){
			loadSectionHeader(child,doc);
		} else if (nodeName.compare("Scales",Qt::CaseInsensitive) == 0){
			loadScalesHeader(child,doc);
		} else if (nodeName.compare("Rows",Qt::CaseInsensitive) == 0){
			loadRows(child,doc);
		} else if (nodeName.compare("Fonts",Qt::CaseInsensitive) == 0){
			loadFont(child,doc);
		}

		child = child.nextSiblingElement();
	}

	m_doc = NULL;

	return retVal;
}

} //namespace uoReport
