/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uorReportAreas.h"
#include <QBuffer>

namespace uoReport {

uoImageData::uoImageData():m_data(0), m_width(-1), m_height(-1), m_format(QImage::Format_Mono)
{}

uoImageData::uoImageData(uchar * data, int width, int height, QImage::Format format)
	:m_data(data), m_width(width), m_height(height), m_format(format)
{
}

uoImageData::uoImageData(const QImage& img, const int& w, const int& h, QImage::Format format)
{
    Q_UNUSED(format);
	int numBytes = img.numBytes();
	const uchar *image_data = img.bits();
	m_width = w;
	m_height = h;
	m_data = new uchar[numBytes];//	m_data = new uchar[(w+1)*(h+1)];
	for (int yy = 0; yy<numBytes; yy++){
		m_data[yy] = image_data[yy];
	}
}
uoImageData::~uoImageData()
{
	if (m_data)
		delete[] m_data;
}
uoCellPictCasher::uoCellPictCasher()
{}
uoCellPictCasher::~uoCellPictCasher()
{}

bool uoCellPictCasher::hasPixmap(const QString& adress) const
{
	return m_cellTextCashe.contains(adress);
}

uoImageData* uoCellPictCasher::getPixmap(const QString& adress) const
{
	if (!m_cellTextCashe.contains(adress))
		return 0;
	return m_cellTextCashe.value(adress);
}

void uoCellPictCasher::setPixmap(const QString& adress, uoImageData* pixmap)
{
	if (m_cellTextCashe.contains(adress)){
		uoImageData* pxmp = m_cellTextCashe.take(adress);
		delete pxmp;
		pxmp = 0;
	}
	m_cellTextCashe.insert(adress, pixmap);
}


void uoCellPictCasher::pixmapClear(const int row/* = -1*/, const int col /* = -1*/)
{
//	Q_ASSERT(row);
//	Q_ASSERT(col);
	if (!m_cellTextCashe.isEmpty()) {
		QString address;
		uoImageData* pxmp = 0;
		if (row > 0 && col > 0) {
			address = QString("%1x%2").arg(row).arg(col);
			if (m_cellTextCashe.contains(address)){
				pxmp = m_cellTextCashe.take(address);
				delete pxmp;
				pxmp = 0;
				m_cellTextCashe.remove(address);
			}
		} else {
			int tmpRow = 0, tmpCol = 0;
			QStringList list;
			bool toErase = false;
			QHash<QString, uoImageData*>::iterator i = m_cellTextCashe.begin();
			while (i != m_cellTextCashe.end()) {
				list = i.key().split('x');
				toErase = false;
				if (list.size() == 2) {
					tmpRow = list.at(0).toInt();
					tmpCol = list.at(1).toInt();
					if (row > 0 && row == tmpRow) {
						toErase = true;
					} else
					if (col > 0 && col == tmpCol)
						toErase = true;
				}
				if (toErase) {
					pxmp = i.value();
					delete pxmp;
					pxmp = 0;
					i = m_cellTextCashe.erase(i);
				} else {
					++i;
				}
			}
		}
	}
}

/// Очистим все
void uoCellPictCasher::pixmapClearAllOutside(const QRect& rect)
{
	if (m_cellTextCashe.isEmpty())
		return;
	int tmpRow = 0, tmpCol = 0;
	QStringList list;
	uoImageData* pxmp = 0;
	bool toErase = false;
	QRect rect2 = rect;
	///\todo надо бы подзасолить пиксмапов так, на будующее....
	if (rect.top()>20) 	{	rect2.adjust(0,-20,0,0); } else {rect2.adjust(0,-rect.top()+1,0,0);}
	if (rect.left()>20)	{ 	rect2.adjust(-20,0,0,0); } else {rect2.adjust(-rect.left()+1,0,0,0); }
	rect2.adjust(0,0,20,20);
	Q_ASSERT(rect2.isValid());

	QHash<QString, uoImageData*>::iterator i = m_cellTextCashe.begin();
	while (i != m_cellTextCashe.end()) {
		list = i.key().split('x');
		toErase = false;
		if (list.size() == 2) {
			tmpRow = list.at(0).toInt();
			tmpCol = list.at(1).toInt();

			toErase = !rect2.contains(tmpCol, tmpRow);
		}
		if (toErase) {
			pxmp = i.value();
			delete pxmp;
			pxmp = 0;
			i = m_cellTextCashe.erase(i);
		} else {
			++i;
		}
	}
}

/// Очистим все
void uoCellPictCasher::pixmapClearAll()
{
	uoImageData* pxmp = 0;
	if (m_cellTextCashe.isEmpty())
		return;

	QHash<QString, uoImageData*>::iterator i = m_cellTextCashe.begin();
	while (i != m_cellTextCashe.end()) {
		pxmp = i.value();
		delete pxmp;
		pxmp = 0;
		i = m_cellTextCashe.erase(i);
	}
}

/// Очистим начиная со строки/столбца и далее по строке столбцу.
void uoCellPictCasher::clearFromToEnd(uoRptHeaderType rht, int from)
{
	int tmpRow = 0, tmpCol = 0;
	QStringList list;
	uoImageData* pxmp = 0;
	bool toErase = false;
	QHash<QString, uoImageData*>::iterator i = m_cellTextCashe.begin();
	while (i != m_cellTextCashe.end()) {
		list = i.key().split('x');
		toErase = false;
		if (list.size() == 2) {
			tmpRow = list.at(0).toInt();
			tmpCol = list.at(1).toInt();
			if (rht == uorRhtRowsHeader && from>= tmpRow) {
				toErase = true;
			} else
			if (rht == uorRhtColumnHeader && from>= tmpCol)
				toErase = true;
		}
		if (toErase) {
			pxmp = i.value();
			delete pxmp;
			pxmp = 0;
			i = m_cellTextCashe.erase(i);
		} else {
			++i;
		}
	}
}



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

QRect uorReportAreaBase::visualRect() const
{
	return QRect(m_firstVisible_ColLeft, m_firstVisible_RowTop, m_lastVisibleCol, m_lastVisibleRow);
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
