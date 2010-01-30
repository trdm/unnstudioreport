/**************************************
*
*	© trdm, as Troshin D. V. 2010
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOPAINTER_H
#define UOPAINTER_H

#include <QPainter>

//namespace uoReport {

class uoPainter : public QPainter
{
	public:
		/** Default constructor */
		uoPainter();
		uoPainter(QPaintDevice *);
		/** Default destructor */
		virtual ~uoPainter();
	protected:
	private:
};
//} //namespace uoReport {

#endif // UOPAINTER_H
