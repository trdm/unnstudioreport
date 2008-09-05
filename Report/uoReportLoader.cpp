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
	_docFilePath = fileName;
}

/// Проверим установлены ли проперти для сохранения/восстановления данных.
bool uoReportLoader::validateStoreProps(bool forLoad){

	if (_docFilePath.isEmpty() ||
		_storeFormat == uoRsf_Unknown)
	{
		return false;
	}
	if (forLoad) {
		/// при чтении.
		if (!QFile::exists(_docFilePath)){
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
	Структура выгрузки в XML

*/
uoReportLoaderXML::uoReportLoaderXML(){
	setFormat(uoRsf_XML);
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
	outStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	_textStream << outStr;
	_textStream << QString("<Doc type = \"%1\" version = \"%2\">\n").arg(UORPT_XMLTYPEDOC).arg(uoReportVersion);
	_textStream << QString("<!-- Scales and Group types \"%1\"=Vertical  \"%2\"=Horizontal -->\n").arg(rhtVertical).arg(rhtHorizontal);
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
		.arg(doc->getDefScaleSize(rhtVertical))
		.arg(doc->getDefScaleSize(rhtHorizontal));
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
	_textStream << QString("\t<Section start = \"%1\" end = \"%2\" name=\"%3\"/>\n")
	.arg(peSpn->getStart()).arg(peSpn->getEnd()).arg(peSpn->_name);
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
	_textStream << QString("\t<Scale no = \"%1\" hide = \"%2\" size=\"%3\"/>\n")
	.arg(rLine->number()).arg(rLine->hiden()).arg(rLine->size());
	return true;
}
bool uoReportLoaderXML::saveScaleHeaderEnd(uoRptHeaderType rht){
	_textStream << QString("</Scales>\n");
	return true;
}

/// Запись картинки
bool uoReportLoaderXML::saveImageStart(QImage* imagedoc, qreal left, qreal top){
    _textStream << QString("<Image left = \"%1\" top = \"%2\" width = \"%3\" height = \"%4\">\n").arg(left).arg(top).arg(imagedoc->width()).arg(imagedoc->height());
    return true;
}

/// Запись происходит с использованием base64
/// чтение необходимо осуществлять с помощью функции:
///     fromBase64 (const QByteArray & base64 )
bool uoReportLoaderXML::saveImageItem(QImage* imagedoc){
    // Сохраняем все в формате PNG (оптимальное сжатие + оптимальное качество)
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    imagedoc->save(&buffer, "PNG");
    _textStream << ba.toBase64();
    return true;
}

bool uoReportLoaderXML::saveImageEnd(QImage* imagedoc){
    _textStream << QString("\n</Image>\n");
    return true;
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

} //namespace uoReport
