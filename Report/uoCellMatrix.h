/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOCELLMATRIX_H
#define UOCELLMATRIX_H

#include "uoReport.h"
struct uoCell;
#include <QMap>

namespace uoReport {


class uoCellMatrix
{
	public:
		uoCellMatrix(int row, int col);
		virtual ~uoCellMatrix();

		void setCell(int row, int col, uoCell* cell);
		void setCellSelected(int row, int col);
		uoCell* cell(int row, int col) const;

		inline int rows() {return m_row; };
		inline int cols() {return m_col; };

	protected:

	private:
		uoCellMatrix();
	private:
		int m_row;
		int m_col;
		int m_size;
		QMap<int,uoCell*> m_matrix;
		QList<int> m_selectedItem;
};

} //namespace uoReport {
#endif // UOCELLMATRIX_H
