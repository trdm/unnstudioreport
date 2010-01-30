/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uorMimeData.h"



namespace uoReport {

uorMimeData::uorMimeData()
{	}
uorMimeData::uorMimeData(uoReportDoc* doc, uoReportSelection* selection)
	:  QMimeData()
	, m_doc(doc)
	, m_selection(selection)
{
	//UOR_MIME_XML_DATA >> "uoReportXmlData"
	m_formats << "text/plain" << UOR_MIME_XML_DATA; //"uoReportXmlData";
}

uorMimeData::~uorMimeData()
{}
bool uorMimeData::prepareData()
{
	bool retVal = true;

	return retVal;
}

QStringList uorMimeData::formats() const
{
	return m_formats;
}


} //namespace uoReport {
