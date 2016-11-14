/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReport.h"
#include "uoReportDoc.h"
#include "uoReportPreviewDlg.h"
#include "uorPageSetup.h"
#include "uoReportDrawHelper.h"
#include "uorPagePrintSetings.h"
#include <QtGui>

namespace uoReport {

uoReportPreviewDlg::uoReportPreviewDlg(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	hide();
	m_scaleFactor = 1.0;
	setWindowFlags(Qt::Dialog| Qt::WindowMinimizeButtonHint| Qt::WindowMaximizeButtonHint);

    m_imageLabel = new QLabel;
    m_imageLabel->setBackgroundRole(QPalette::Base);
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel->setScaledContents(true);


	m_scrollArea = new QScrollArea;
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QGridLayout* t_gridLayout = new QGridLayout;
	t_gridLayout->setSpacing(0);
//#if (QT_VERSION > 0x040201)
	t_gridLayout->setHorizontalSpacing(0);
	t_gridLayout->setContentsMargins(0, 0, 0, 0);
//#endif
	m_frame->setLayout(t_gridLayout);
	t_gridLayout->addWidget(m_scrollArea);
	//QObject::
	connect(m_pbPrint,SIGNAL(clicked()),this, SLOT(onBtnPrint()));
	connect(m_pbPage,SIGNAL(clicked()),this, SLOT(onBtnPage()));
	connect(m_pbNext,SIGNAL(clicked()),this, SLOT(onBtnNext()));
	connect(m_pbPrevious,SIGNAL(clicked()),this, SLOT(onBtnPrevious()));
	connect(m_pbZoomIn,SIGNAL(clicked()),this, SLOT(onBtnZoomIn()));
	connect(m_pbZoomOut,SIGNAL(clicked()),this, SLOT(onBtnZoomOut()));
	connect(m_pbNormSize,SIGNAL(clicked()),this, SLOT(onBtnnormalSize()));
	connect(m_pbClose,SIGNAL(clicked()),this, SLOT(onBtnClose()));
	connect(m_leCurentPage,SIGNAL(editingFinished()),this, SLOT(onSetPageNom()));
	m_pageList = NULL;
	m_pagesAll = m_pageCurent = -1;
	updatePageNom();
}

uoReportPreviewDlg::~uoReportPreviewDlg()
{
//	delete m_pageList; // привет, утечка, я тя потом поправлю..
}



void uoReportPreviewDlg::setDoc(uoReportDoc* rptDoc)
{
	if (!rptDoc){
		return;
	}
	m_Doc = rptDoc;
	recalcData();
}
/// Сменились некоторые данные, например параметры страницы, надо все пересчитать.
void uoReportPreviewDlg::recalcData()
{
	m_pagesAll = m_pageCurent = -1;
	if (!m_Doc) {
		return;
	}

	m_pageList = m_Doc->getPageList();

	if (!m_pageList) {
        /// Nagumanov 14.05.2010 Сообщаем пользоватею, что невозможно создать предпросмотр
        QMessageBox::critical(this, tr("Report"), tr("Cant create preview, please check your printer settings."));
        return;
	}
	m_pagesAll = m_pageList->count();
	m_labelTotalPg->setText(QString("/ %1").arg(m_pagesAll));
	if (m_pagesAll>0)
		m_pageCurent = 1;
	updatePageNom();

	m_pageRectPrint = m_Doc->getPageRectPrint();
	m_paperRectPrint = m_Doc->getPaperRectPrint();
	m_imageLabel->resize(m_paperRectPrint.size());
	paintPreview();

}

void uoReportPreviewDlg::paintPreview()
{
	if (!m_Doc) {		return;	}
	if (m_pageList->isEmpty())
		return;
	if (m_pageList->count() != m_pagesAll)
		return;

	uorReportPrintArea* area = NULL;
	uorAresListCnstIter iter = m_pageList->constBegin();
	while(iter != m_pageList->constEnd()){
		area = *iter;
		if (area) {
			if (area->m_pageNumber != m_pageCurent)	{
				area = NULL;
			} else {
				break;
			}
		}
		area = NULL;
		iter++;
	}
	if (!area)
		return;
	QPixmap pixmap(m_paperRectPrint.size());

    uoPainter painter;
    uorPagePrintSetings* pSet =  m_Doc->pagesSetings();
    painter.begin(&pixmap);

    painter.fillRect(m_paperRectPrint, QBrush(Qt::white));

	uoReportDrawHelper* drawHelper = new uoReportDrawHelper(m_Doc);
	drawHelper->initDrawInstruments();

	drawHelper->drawFields(painter);
	qreal scaleFactor = pSet->scale();
//	if(scaleFactor != 1.0)		scaleFactor = 1/scaleFactor;
//	qDebug() << QString("void uoReportPreviewDlg::paintPreview scaleFactor = %1").arg(scaleFactor);
	painter.scale(scaleFactor,scaleFactor);

	uorReportAreaBase aBase = (*area);
	aBase.m_areaType = 2;
    drawHelper->drawDataArea(painter, aBase);
    painter.end();

	m_imageLabel->setPixmap(pixmap);

}


void uoReportPreviewDlg::onCurrentPageChange()
{
	paintPreview();
}

void uoReportPreviewDlg::onBtnPrint() //m_pbPrint
{
	if (!m_Doc)
		return;
	//m_Doc->printDoc(false,m_imageLabel);
    if( !m_Doc->printDoc(false,m_imageLabel) )
    {
        /// Nagumanov 14.05.2010 Сообщаем пользователю, что невозможно распечатать проект
        QMessageBox::critical(this, tr("Report"), tr("Cant print report, please check your printer settings."));
    }


}
void uoReportPreviewDlg::onBtnPage() //m_pbPage
{
	uorPageSetup* pPageStDlg = new uorPageSetup(this);
	pPageStDlg->setSettings(m_Doc->pagesSetings());
	int dret = pPageStDlg->exec();
	Q_UNUSED(dret);

	delete pPageStDlg;
	recalcData();

}
void uoReportPreviewDlg::onBtnNext() //m_pbNext
{
	if (m_pageCurent < m_pagesAll) {
		m_pageCurent += 1;
		updatePageNom();
		paintPreview();
	}
}
void uoReportPreviewDlg::onBtnPrevious() //m_pbPrevious
{
	if (m_pageCurent > 1) {
		m_pageCurent -= 1;
		updatePageNom();
		paintPreview();
	}
}

void uoReportPreviewDlg::updatePageNom()
{
	m_leCurentPage->blockSignals(true);
	m_leCurentPage->setText(QString("%1").arg(m_pageCurent));
	m_leCurentPage->blockSignals(false);

}

void uoReportPreviewDlg::scaleImage(double factor)
{
	Q_ASSERT(m_imageLabel->pixmap());
	m_scaleFactor *= factor;
	m_imageLabel->resize(m_scaleFactor * m_imageLabel->pixmap()->size());

	adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
	adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

	// todo - укправление кнопками
	m_pbZoomIn->setEnabled(m_scaleFactor < 3.0);
	m_pbZoomOut->setEnabled(m_scaleFactor > 0.333);

}

void uoReportPreviewDlg::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
	scrollBar->setValue(int(factor * scrollBar->value()
							+ ((factor - 1) * scrollBar->pageStep()/2)));
}

void uoReportPreviewDlg::onSetPageNom() //editingFinished
{
	int pageCurent = m_leCurentPage->text().toInt();
	if (pageCurent > 0 || pageCurent<= m_pagesAll) {
		if (m_pageCurent != pageCurent) {
			m_pageCurent = pageCurent;
			paintPreview();
		}
	}
}

void uoReportPreviewDlg::onBtnZoomIn() //m_pbZoomIn
{
	scaleImage(1.25);
}
void uoReportPreviewDlg::onBtnZoomOut() //m_pbZoomOut
{
	scaleImage(0.8);
}

void uoReportPreviewDlg::onBtnnormalSize()
{
	m_imageLabel->adjustSize();
	m_scaleFactor = 1.0;

}
void uoReportPreviewDlg::onBtnClose() //m_pbClose
{
	QDialog::reject();
}

} //namespace uoReport
