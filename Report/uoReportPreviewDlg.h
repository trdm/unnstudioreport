/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTPREVIEWDLG_H
#define UOREPORTPREVIEWDLG_H


#include "uoReport.h"
#include "ui_uoReportPreviewDlg.h"
#include "uorReportAreas.h"

class QDialog;
class QLabel;
class QScrollArea;
class uorPageSetup;
class uoReportDoc;

namespace uoReport {


class uoReportPreviewDlg : public QDialog, Ui::uoReportPrevDlg
{
	Q_OBJECT
	public:
		uoReportPreviewDlg(QWidget *parent = 0);
		virtual ~uoReportPreviewDlg();
	public:
		void setDoc(uoReportDoc* rptDoc);

	private slots:
		void onBtnPrint(); //m_pbPrint
		void onBtnPage(); //m_pbPage
		void onBtnNext(); //m_pbNext
		void onBtnPrevious(); //m_pbPrevious
		void onBtnZoomIn(); //m_pbZoomIn
		void onBtnZoomOut(); //m_pbZoomOut
		void onBtnnormalSize(); //m_pbNormSize
		void onBtnClose(); //m_pbClose
		void onSetPageNom();//editingFinished
	private:
		void onCurrentPageChange(); ///< сменилась текущая страница, надо перерисовать лейбу и только тут.
		void recalcData(); 			///< сменились некоторые данные, например параметры страницы, надо все пересчитать.
		void paintPreview();
		void updatePageNom();
		void scaleImage(double factor);
		void adjustScrollBar(QScrollBar *scrollBar, double factor);

	private:
		uoReportDoc* m_Doc;
		QLabel *m_imageLabel;
		QScrollArea *m_scrollArea;
		double m_scaleFactor;

		uorAresList* m_pageList;

		int m_pagesAll, m_pageCurent;

		QRect 	m_pageRectPrint;
		QRect 	m_paperRectPrint;

};

} //namespace uoReport
#endif // UOREPORTPREVIEWDLG_H
