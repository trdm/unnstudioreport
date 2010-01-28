/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UORMIMEDATA_H
#define UORMIMEDATA_H

#include "uoReport.h"
#include <QMimeData>
#include <QStringList>
class QStringList;


namespace uoReport {
//Q_DECLARE_METATYPE(clipb_struct);

class uorMimeData : public QMimeData
{
	public:
		uorMimeData(uoReportDoc* doc, uoReportSelection* selection);
		virtual ~uorMimeData();
		virtual QStringList formats () const;

		bool prepareData();
	protected:
		uorMimeData();
	private:
		uoReportDoc* m_doc;
		uoReportSelection* m_selection;
		QStringList m_formats;
};
} //namespace uoReport {
#endif // UORMIMEDATA_H
