/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include "uoReport.h"
#include "uoReportDoc.h"
#include "uoSpanTree.h"


namespace uoReport {

/**
	\class uoReportTest - класс обобщенного тестирования
	\brief класс для тестирования всех классов в пространстве uoReport
*/
uoReportTest::uoReportTest()
{
}
uoReportTest::~uoReportTest(){
}

/// Общая процедура запуска тестирования классов наймспейса "uoReport"
void uoRunTest(){
    uoSpanTree pST;
    pST.testClass();
    uoHeaderScale header;
    header.test();

    uoReportDoc uorptDoc;
    uorptDoc.test();

}

} // namespace uoReport
