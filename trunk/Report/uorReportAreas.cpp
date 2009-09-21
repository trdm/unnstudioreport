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

void uorReportAreaBase::clear()
{
	m_shift_ColLeft = m_shift_RowTop = 0.0;
    m_firstVisible_RowTop = m_firstVisible_ColLeft = 0;
	m_lastVisibleRow = m_lastVisibleCol = 0;
	m_area = QRectF(0.0, 0.0, 0.0, 0.0);
	m_areaType = 1;

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
	, m_offsetStart(0.0)
	, m_segmWidth(0.0)
{}

uorPageColSegment::uorPageColSegment(int segmNo, int colS, int colN)
	: m_segmentNom(segmNo)
	, m_colStart(colS)
	, m_colEnd(colN)
	, m_offsetStart(0.0)
	, m_segmWidth(0.0)
{}
uorPageColSegment::~uorPageColSegment()
{}



uorReportViewArea::uorReportViewArea()
	:uorReportAreaBase()
	, m_groupList(new uoRptGroupItemList)
	,m_sectItemList(new uoRptSectionItemList)
{
	clear();
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

} // namespace uoReport
