#ifndef UOREPORTCOMMON_H_INCLUDED
#define UOREPORTCOMMON_H_INCLUDED

/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

//#include "uoReport.h"

namespace uoReport {

#include "uoReportDescr.h"

// На глазок вроде ничего..
// (примечание) Scale - ячейка линейки, Cell - ячейка основного поля, \n чисто для изучения английского :)
#define UORPT_SCALE_SIZE_DEF_VERTICAL 	15.0
#define UORPT_SCALE_SIZE_DEF_HORIZONTAL 60.0

#define uoReportVersion "0.1"
#define rptSize qreal
#define rptSizeNull 0.0

#define rptRect QRectF


struct uoLineSpan;
struct uoRptNumLine;
class uoSpanTree;
class uoReportDoc;
class uoReportCtrl;
class uoReportView;
class uoReportViewIteract;
class uoReportDocBody;
template <typename T> class uoNumVector;
class uoReportLoader;
class uoReportLoaderXML;


///\enum uoIntersectMode - варианты перечения отрезков
typedef enum uoIntersectMode {
	  ismNone 		= 0
	, ismAbove 		= 1
	, ismAboveIn 	= 3
	, ismIn 		= 2
	, ismInBelow 	= 6
	, ismBelow 		= 4
	, ismSurround 	= 7
	, ismAboveUp 	= 9
	, ismBelowDown 	= 12
};
typedef enum uoRptHeaderType {
	  rhtUnknown = 0
	, rhtVertical = 1
	, rhtHorizontal = 2
};

typedef enum uoSearchDirection {
	toUp,	///< направление поиска вверх
	toDown	///< направление поиска ВНИЗ
};

typedef enum uoScaleSizePolicy
{
	uoSSP_fixed = 0 ///< расширения не происходит
	, uoSSP_minimum ///< текущий размер минимально возможный, т.е. ячейка может расширяться.
	, uoSSP_maximum ///< установленный размер является максимальным, т.е. ячейка будет стремиться сужаться.
};

///\enum uoRptStoreFormat - Формат сохранения отчета.
typedef enum uoRptStoreFormat {
	  uoRsf_Unknown = 0
	, uoRsf_XML = 1
	, uoRsf_Binary = 2
	, uoRsf_HTML = 3
};
///spares - запчасти ; запасные части; spare parts, spares
/// пригодятся когда буду выгружать в бинарный формат.
typedef enum uoRptDocSparesType {
/// Dst - (DocSparesType)
	  uoDst_defFontName = 10
	, uoDst_defFontSize
};

/// Запуск тестов для элементов отчета...
extern void uoRunTest();

class uoReportTest {
	uoReportTest();
	~uoReportTest();
};

} // namespace uoReport


#endif // UOREPORTCOMMON_H_INCLUDED
