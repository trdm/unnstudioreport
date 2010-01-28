/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoCellMatrix.h"
#include "uoReportDocBody.h"

namespace uoReport {

uoCellMatrix::uoCellMatrix()
{}

uoCellMatrix::uoCellMatrix(int row, int col)
{
	Q_ASSERT(row);
	Q_ASSERT(col);
	m_row = row;
	m_col = col;
	m_size = m_row * m_col;
}



uoCellMatrix::~uoCellMatrix()
{}

void uoCellMatrix::setCell(const int row, const int col, uoCell* cell)
{
	Q_ASSERT(row);
	Q_ASSERT(col);

	int index = (row-1)*m_col + col;
	m_matrix.insert(index, cell);
}

void uoCellMatrix::setCellSelected(int row, int col)
{
	Q_ASSERT(row);
	Q_ASSERT(col);
	int sz = row*col;
	int index = (row-1)*m_col + col;
	if ( m_size >= sz){
		m_selectedItem.append(index);
	}

}

uoCell* uoCellMatrix::cell(int row, int col)  const
{
	uoCell* cell = 0;
	Q_ASSERT(row);
	Q_ASSERT(col);
	int sz = row*col;
	int index = (row-1)*m_col + col;
	if ( m_size >= sz){

		if (m_matrix.contains(index)){
			cell = m_matrix.value(index);
		}
	}
	return cell;
}

} //namespace uoReport {
