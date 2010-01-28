/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uorReportAreas.h"

namespace uoReport {


uorReportAreaBase::uorReportAreaBase()
{
	clear();
}
uorReportAreaBase::~uorReportAreaBase()
{}

void uorReportAreaBase::setShift_RowTop(const uorNumber& val) 	{
	if (int(m_shift_RowTop) != int(val))
		change();
	m_shift_RowTop = val;
}
void uorReportAreaBase::setShift_ColLeft(const uorNumber& val) 	{
	if (int(m_shift_ColLeft) != int(val))
		change();
	m_shift_ColLeft = val;
}

void uorReportAreaBase::setFirstVisible_RowTop(const int& val ) {
	if(val != m_firstVisible_RowTop)
		change();
	m_firstVisible_RowTop = val;
}

void uorReportAreaBase::setFirstVisible_ColLeft(const int& val ) {
	if (m_firstVisible_ColLeft != val)
		change();
	m_firstVisible_ColLeft = val;
}

void uorReportAreaBase::setLastVisibleRow(const int& val ) {
	if (m_lastVisibleRow != val)
		change();
	m_lastVisibleRow = val;
}

void uorReportAreaBase::setLastVisibleCol(const int& val ) {
	if (m_lastVisibleCol != val)
		change();
	m_lastVisibleCol = val;
}

void uorReportAreaBase::copyTo(uorReportAreaBase& target){
	target.m_area 		= m_area;
	target.m_areaType 	= m_areaType;
	target.m_changes 	= m_changes;
	target.m_shift_RowTop 		= m_shift_RowTop;
	target.m_shift_ColLeft 		= m_shift_ColLeft;
	target.m_firstVisible_RowTop= m_firstVisible_RowTop;
	target.m_firstVisible_ColLeft= m_firstVisible_ColLeft;
	target.m_lastVisibleRow 	= m_lastVisibleRow;
	target.m_lastVisibleCol 	= m_lastVisibleCol;
}

void uorReportAreaBase::clear()
{
	m_shift_ColLeft = m_shift_RowTop = uorNumberNull;
    m_firstVisible_RowTop = m_firstVisible_ColLeft = 0;
	m_lastVisibleRow = m_lastVisibleCol = 0;
	m_area = uorRect(uorNumberNull, uorNumberNull, uorNumberNull, uorNumberNull);
	m_areaType = 1;
	m_changes = 0;

}

uorReportPrintArea::uorReportPrintArea()
	:uorReportAreaBase()
	, m_colOverStart(0)
	, m_colOverEnd(0)
	, m_pageNumber(0)
	, m_pageColumn(0)
	, m_segment(0)
{
	m_areaType = 1;
}

uorReportPrintArea::~uorReportPrintArea()
{
	m_segment = NULL;
}


uorPageColSegment::uorPageColSegment()
	: m_segmentNom(0)
	, m_colStart(0)
	, m_colEnd(0)
	, m_offsetStart(uorNumberNull)
	, m_segmWidth(uorNumberNull)
{}

uorPageColSegment::uorPageColSegment(int segmNo, int colS, int colN)
	: m_segmentNom(segmNo)
	, m_colStart(colS)
	, m_colEnd(colN)
	, m_offsetStart(uorNumberNull)
	, m_segmWidth(uorNumberNull)
{}
uorPageColSegment::~uorPageColSegment()
{}



uorReportViewArea::uorReportViewArea()
	:uorReportAreaBase()
	, m_groupList(new uoRptGroupItemList)
	,m_sectItemList(new uoRptSectionItemList)
{
	clear();
	m_changes = -1;
}
uorReportViewArea::uorReportViewArea(const uorReportViewArea& re)
	:uorReportAreaBase()
{
	m_shift_RowTop = re.m_shift_RowTop;
	m_shift_ColLeft = re.m_shift_ColLeft;

	m_firstVisible_RowTop = re.m_firstVisible_RowTop;
	m_firstVisible_ColLeft = re.m_firstVisible_ColLeft; 	///< Первая левая видимая колонка

	m_lastVisibleRow = re.m_lastVisibleRow; 	///< Последняя верхняя видимая строка
	m_lastVisibleCol = re.m_lastVisibleCol; 	///< Последняя левая видимая колонка

	m_area  = re.m_area;
}

uorReportViewArea& uorReportViewArea::operator=(uorReportViewArea& re)
{
	m_shift_RowTop = re.m_shift_RowTop;
	m_shift_ColLeft = re.m_shift_ColLeft;

	m_firstVisible_RowTop = re.m_firstVisible_RowTop;
	m_firstVisible_ColLeft = re.m_firstVisible_ColLeft; 	///< Первая левая видимая колонка

	m_lastVisibleRow = re.m_lastVisibleRow; 	///< Последняя верхняя видимая строка
	m_lastVisibleCol = re.m_lastVisibleCol; 	///< Последняя левая видимая колонка

	m_area  = re.m_area;
	return *this;
}

uorReportViewArea::~uorReportViewArea()
{
	delete m_groupList;
	delete m_sectItemList;
}
void uorReportViewArea::clear()
{
	uorReportAreaBase::clear();
}

QString uorReportViewArea::toDebug()
{
	QString str = QString(
	"first_Row = %1 last_Row = %3 "
	"first_Col = %2 last_Col = %4 "
	"shift_RowTop = %5 shift_ColLeft = %6")
	.arg(m_firstVisible_RowTop)
	.arg(m_firstVisible_ColLeft)
	.arg(m_lastVisibleRow)
	.arg(m_lastVisibleCol)
	.arg(m_shift_RowTop)
	.arg(m_shift_ColLeft)
	;
	return str;
}
} // namespace uoReport
