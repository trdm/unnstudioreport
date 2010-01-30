/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include <QDialog>
#include "uoReport.h"
#include "uorPageSetup.h"
#include "uorPagePrintSetings.h"

namespace uoReport {

uorPageSetup::uorPageSetup(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	hide();
	m_pagesSetings = NULL;
	connect(m_rbPortrait, SIGNAL(clicked()), this,SLOT(refreshPageLabel()));
	connect(m_rbLandscape, SIGNAL(clicked()), this,SLOT(refreshPageLabel()));
	connect(m_rbScalePunctual, SIGNAL(clicked()), this,SLOT(refreshPageLabel()));
	connect(m_rbScalePunctual2, SIGNAL(clicked()), this,SLOT(refreshPageLabel()));
	populateCombo();
}


uorPageSetup::~uorPageSetup()
{
	//dtor
}


void uorPageSetup::populateCombo()
{
    m_cbPaperSize->addItem(tr("A0 (841 x 1189 mm)"), QPrinter::A0);
    m_cbPaperSize->addItem(tr("A1 (594 x 841 mm)"), QPrinter::A1);
    m_cbPaperSize->addItem(tr("A2 (420 x 594 mm)"), QPrinter::A2);
    m_cbPaperSize->addItem(tr("A3 (297 x 420 mm)"), QPrinter::A3);
    m_cbPaperSize->addItem(tr("A4 (210 x 297 mm, 8.26 x 11.7 inches)"), QPrinter::A4);
    m_cbPaperSize->addItem(tr("A5 (148 x 210 mm)"), QPrinter::A5);
    m_cbPaperSize->addItem(tr("A6 (105 x 148 mm)"), QPrinter::A6);
    m_cbPaperSize->addItem(tr("A7 (74 x 105 mm)"), QPrinter::A7);
    m_cbPaperSize->addItem(tr("A8 (52 x 74 mm)"), QPrinter::A8);
    m_cbPaperSize->addItem(tr("A9 (37 x 52 mm)"), QPrinter::A9);
    m_cbPaperSize->addItem(tr("B0 (1000 x 1414 mm)"), QPrinter::B0);
    m_cbPaperSize->addItem(tr("B1 (707 x 1000 mm)"), QPrinter::B1);
    m_cbPaperSize->addItem(tr("B2 (500 x 707 mm)"), QPrinter::B2);
    m_cbPaperSize->addItem(tr("B3 (353 x 500 mm)"), QPrinter::B3);
    m_cbPaperSize->addItem(tr("B4 (250 x 353 mm)"), QPrinter::B4);
    m_cbPaperSize->addItem(tr("B5 (176 x 250 mm, 6.93 x 9.84 inches)"), QPrinter::B5);
    m_cbPaperSize->addItem(tr("B6 (125 x 176 mm)"), QPrinter::B6);
    m_cbPaperSize->addItem(tr("B7 (88 x 125 mm)"), QPrinter::B7);
    m_cbPaperSize->addItem(tr("B8 (62 x 88 mm)"), QPrinter::B8);
    m_cbPaperSize->addItem(tr("B9 (44 x 62 mm)"), QPrinter::B9);
    m_cbPaperSize->addItem(tr("B10 (31 x 44 mm)"), QPrinter::B10);
    m_cbPaperSize->addItem(tr("C5E (163 x 229 mm)"), QPrinter::C5E);
    m_cbPaperSize->addItem(tr("DLE (110 x 220 mm)"), QPrinter::DLE);
//    m_cbPaperSize->addItem(tr("Executive (7.5 x 10 inches, 191 x 254 mm)"), QPrinter::Executive);
    m_cbPaperSize->addItem(tr("Folio (210 x 330 mm)"), QPrinter::Folio);
    m_cbPaperSize->addItem(tr("Ledger (432 x 279 mm)"), QPrinter::Ledger);
    m_cbPaperSize->addItem(tr("Legal (8.5 x 14 inches, 216 x 356 mm)"), QPrinter::Legal);
    m_cbPaperSize->addItem(tr("Letter (8.5 x 11 inches, 216 x 279 mm)"), QPrinter::Letter);
    m_cbPaperSize->addItem(tr("Tabloid (279 x 432 mm)"), QPrinter::Tabloid);
    m_cbPaperSize->addItem(tr("US Common #10 Envelope (105 x 241 mm)"), QPrinter::Comm10E);
    m_cbPaperSize->setCurrentIndex(m_cbPaperSize->findData(QPrinter::A4));

}

void uorPageSetup::refreshPageLabel()
{
	m_llPageViewL->hide();
	m_llPageViewP->hide();
	m_leScalePunctual->setEnabled(m_rbScalePunctual->isChecked() ? true : false);
	if (m_rbPortrait->isChecked()) 	{
		m_llPageViewP->show();
	} else {
		m_llPageViewL->show();
	}
}

void uorPageSetup::setSettings(uorPagePrintSetings* pSetings)
{
	if (!pSetings)
		return;
	m_pagesSetings = pSetings;
	m_cbPaperSize->setCurrentIndex(m_cbPaperSize->findData(pSetings->m_paperSize));
	m_rbPortrait->setChecked(false);
	m_rbLandscape->setChecked(false);
	if (pSetings->m_orientation == QPrinter::Portrait) {m_rbPortrait->setChecked(true);}
	if (pSetings->m_orientation == QPrinter::Landscape) {m_rbLandscape->setChecked(true);}
	m_rbScalePunctual->setChecked(false);
	m_rbScalePunctual2->setChecked(false);
	m_leScalePunctual->setText(QString("%1").arg(pSetings->m_scopeVal));
	if (pSetings->m_scopeType == 1) {m_rbScalePunctual->setChecked(true);}
	if (pSetings->m_scopeType == 2) {m_rbScalePunctual2->setChecked(true);}

	m_leFieldsLeft->setText(QString("%1").arg(pSetings->m_fieldLeft));
	m_leFieldsTop->setText(QString("%1").arg(pSetings->m_fieldTop));
	m_leFieldsRight->setText(QString("%1").arg(pSetings->m_fieldRight));
	m_leFieldsBottom->setText(QString("%1").arg(pSetings->m_fieldBottom));

	m_leRTitleTop->setText(QString("%1").arg(pSetings->m_titleTopSize));
	m_leRTitleBottom->setText(QString("%1").arg(pSetings->m_titleBotSize));

	refreshPageLabel();
}

void uorPageSetup::accept()
{
	m_pagesSetings->m_paperSize = (QPrinter::PageSize)m_cbPaperSize->itemData(m_cbPaperSize->currentIndex()).toInt();
	m_pagesSetings->m_orientation = (QPrinter::Orientation) m_rbPortrait->isChecked() ? QPrinter::Portrait : QPrinter::Landscape;
	m_pagesSetings->m_fieldLeft = m_leFieldsLeft->text().toInt();
	m_pagesSetings->m_fieldTop = m_leFieldsTop->text().toInt();
	m_pagesSetings->m_fieldRight  = m_leFieldsRight->text().toInt();
	m_pagesSetings->m_fieldBottom = m_leFieldsBottom->text().toInt();

//	m_pagesSetings->m_scopeType = m_rbScalePunctual->isChecked() ? 1 : 2;
//	m_pagesSetings->m_scopeVal = m_leScalePunctual->text().toInt();
	m_pagesSetings->setScope(m_rbScalePunctual->isChecked() ? 1 : 2 , m_leScalePunctual->text().toInt());


	m_pagesSetings->m_titleTopSize = m_leRTitleTop->text().toInt();
	m_pagesSetings->m_titleBotSize = m_leRTitleBottom->text().toInt();

	QDialog::accept();
}

} //namespace uoReport
