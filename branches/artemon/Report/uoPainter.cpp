/**************************************
*
*	© trdm, as Troshin D. V. 2010
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoPainter.h"

//namespace uoReport {

uoPainter::uoPainter()
{	//ctor
}
uoPainter::uoPainter(QPaintDevice * pd)
	:QPainter(pd)
{	//ctor
}


uoPainter::~uoPainter()
{
	//dtor
}

//} //namespace uoReport {
