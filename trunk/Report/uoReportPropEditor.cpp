/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include "uoReportPropEditor.h"
#include "uoReportDocBody.h"
#include <QDesktopWidget>


namespace uoReport {

uoTextPropTab::uoTextPropTab(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	hide();
	m_textCell->setAcceptRichText(false);
}

uoTextPropTab::~uoTextPropTab()
{}

void uoTextPropTab::init()
{
	this->m_cbTextType->addItem(QString::fromUtf8(""),uoCTT_Unknown);
	this->m_cbTextType->addItem(QString::fromUtf8("Text"),uoCTT_Text);
	this->m_cbTextType->addItem(QString::fromUtf8("Expr"),uoCTT_Expr);
	this->m_cbTextType->addItem(QString::fromUtf8("Templ"),uoCTT_Templ);

	this->m_cbTextBehavior->addItem(QString::fromUtf8(""),uoCTB_Unknown);
	this->m_cbTextBehavior->addItem(QString::fromUtf8("Auto"),uoCTB_Auto);
	this->m_cbTextBehavior->addItem(QString::fromUtf8("Cut"),uoCTB_Cut);
	this->m_cbTextBehavior->addItem(QString::fromUtf8("Obstruct"),uoCTB_Obstruct);
	this->m_cbTextBehavior->addItem(QString::fromUtf8("Transfer"),uoCTB_Transfer);
}

/// Считываем данные из m_rpe->m_textProp
void uoTextPropTab::initFromRPE()
{
	// в списках типа текста и поведения иногда необъодимы итемы ункноун, а иногда нет.
	int index = m_cbTextType->findData(uoCTT_Unknown);
	if (m_rpe->m_textProp->m_textType != uoCTT_Unknown && index != -1){
		m_cbTextType->removeItem ( index );
	}
	index = m_cbTextType->findData(m_rpe->m_textProp->m_textType);
	if (index == -1) {
		m_cbTextType->insertItem(0,QString::fromUtf8(""),uoCTT_Unknown);
	} else {
		m_cbTextType->setCurrentIndex(index);
	}


	index = m_cbTextBehavior->findData(uoCTB_Unknown);

	if (m_rpe->m_textProp->m_TextBehavior != uoCTB_Unknown && index != -1){
		m_cbTextBehavior->removeItem ( index );
	}
	index = m_cbTextBehavior->findData(m_rpe->m_textProp->m_TextBehavior);
	if (index == -1) {
		m_cbTextBehavior->insertItem(0,QString::fromUtf8(""),uoCTB_Unknown);
	} else {
		m_cbTextBehavior->setCurrentIndex(index);
	}

	bool permTxt = (m_rpe->m_sellectonType == uoRst_Unknown)? true : false;

	m_textCell->setEnabled(permTxt);
	m_textDecoding->setEnabled(permTxt);
	if (permTxt ){
		m_textCell->setPlainText(m_rpe->m_cellText);
		m_textDecoding->setText(m_rpe->m_cellDecode);
	}
}

/// Заливаем данные из гуя в проперть.
void uoTextPropTab::applyResult()
{
	m_rpe->m_textPropCopy->m_textType 		= (uoCellTextType)m_cbTextType->itemData(m_cbTextType->currentIndex()).toInt();
	m_rpe->m_textPropCopy->m_TextBehavior	= (uoCellTextBehavior)m_cbTextBehavior->itemData(m_cbTextBehavior->currentIndex()).toInt();
	m_rpe->m_cellText = m_textCell->toPlainText();
	m_rpe->m_cellDecode = m_textDecoding->text();

}

uoTextLayotTab::uoTextLayotTab(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	hide();
	m_grVA = new QButtonGroup(this);
	m_grVA->addButton(m_TopVTA);
	m_grVA->addButton(m_CenterVTA);
	m_grVA->addButton(m_BottomVTA);

	m_grHA = new QButtonGroup(this);
	m_grHA->addButton(m_LeftHTA);
	m_grHA->addButton(m_CenterHTA);
	m_grHA->addButton(m_RightHTA);
	chbUseSelectedCell->setDisabled(true);

}
uoTextLayotTab::~uoTextLayotTab()
{}

void uoTextLayotTab::initFromRPE()
{
	m_grVA->setExclusive(false);
	m_grHA->setExclusive(false);

	m_TopVTA->setChecked(false);
	m_CenterVTA->setChecked(false);
	m_BottomVTA->setChecked(false);

	m_LeftHTA->setChecked(false);
	m_CenterHTA->setChecked(false);
	m_RightHTA->setChecked(false);
	if (m_rpe->m_textProp){
		switch (m_rpe->m_textProp->m_horTAlignment){
		case uoHA_Left: 	{m_LeftHTA->setChecked(true); break;}
		case uoHA_Center: 	{m_CenterHTA->setChecked(true); break;}
		case uoHA_Right: 	{m_RightHTA->setChecked(true); break;}
		default:
			break;
		}
		switch (m_rpe->m_textProp->m_vertTAlignment){
		case uoVA_Top: 		{m_TopVTA->setChecked(true); break;}
		case uoVA_Center: 	{m_CenterVTA->setChecked(true); break;}
		case uoVA_Bottom: 	{m_BottomVTA->setChecked(true); break;}
		default:
			break;
		}
	}
	m_grVA->setExclusive(true);
	m_grHA->setExclusive(true);

}

void uoTextLayotTab::applyResult()
{
	if (!m_rpe)
		return;
	if (!m_rpe->m_textPropCopy)
		return;

	uoHorAlignment horTAlignment = uoHA_Unknown;
	if (m_LeftHTA->isChecked())		{		horTAlignment = uoHA_Left;		}
	if (m_CenterHTA->isChecked())	{		horTAlignment = uoHA_Center;	}
	if (m_RightHTA->isChecked())	{		horTAlignment = uoHA_Right;	}
	if (horTAlignment != m_rpe->m_textPropCopy->m_horTAlignment)
	{
		m_rpe->m_textPropCopy->m_horTAlignment = horTAlignment;
	}
	uoVertAlignment vTAlignment = uoVA_Unknown;
	if (m_TopVTA->isChecked())		{		vTAlignment = uoVA_Top;		}
	if (m_CenterVTA->isChecked())	{		vTAlignment = uoVA_Center;	}
	if (m_BottomVTA->isChecked())	{		vTAlignment = uoVA_Bottom;	}
	if (vTAlignment != m_rpe->m_textPropCopy->m_vertTAlignment)
	{
		m_rpe->m_textPropCopy->m_vertTAlignment = vTAlignment;
	}
}


uoTextFontPropTab::uoTextFontPropTab(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	m_ColChooser = new uoColorChooser(m_colFrm);
	QHBoxLayout* layot = new QHBoxLayout(m_colFrm);
	layot->addWidget(m_ColChooser);
    layot->setSpacing(0);
    layot->setMargin(0);
	hide();
}

uoTextFontPropTab::~uoTextFontPropTab()
{}

void uoTextFontPropTab::init()
{
	m_listFonts->clear();
	if(!m_rpe)
		return;
	m_listFntNames = m_rpe->m_fontBD.families(/*m_rpe->m_writingSystem*/);
	for (int i = 0; i<m_listFntNames.count(); i++){
		m_listFonts->addItem(m_listFntNames.at(i));
	}
	m_listFntSizes = m_rpe->m_fontBD.standardSizes();
	connect(m_listFonts, SIGNAL(currentTextChanged(const QString&)),m_fontName,SLOT(setText( const QString & )));
	connect(m_fontName, SIGNAL(textChanged(const QString&)),this,SLOT(onFontNameChange( const QString & )));

	connect(m_sizeList, SIGNAL(currentTextChanged(const QString&)),m_size,SLOT(setText( const QString & )));

	connect(m_fontName, SIGNAL(editingFinished()),this,SLOT(onFontNameEditFinish()));
	m_cmpl = new QCompleter(m_listFntNames, this);

	if (m_cmpl){
		m_fontName->setCompleter(m_cmpl);
		m_cmpl->setCaseSensitivity(Qt::CaseInsensitive);
	}
	m_fontName->installEventFilter(this);
	m_size->installEventFilter(this);
}

void setupCheckState(int state, QCheckBox* chb)
{
	if (state>=0) {
		if (state == 0)
			chb->setCheckState(Qt::Unchecked);
		else
			chb->setCheckState(Qt::Checked);
	} else {
		chb->setCheckState(Qt::PartiallyChecked);
	}

}

void uoTextFontPropTab::initFromRPE()
{
	if (!m_rpe->m_textProp)
		return;
	uoReportDoc* doc = m_rpe->getDoc();
	if (!doc)
		return;
	QString str;
	QFont* font = doc->getFontByID(m_rpe->m_textProp->m_fontId);
	m_fontName->setText("");
	if (font){
		m_fontName->setText(font->family());
	} else {
		m_listFonts->setCurrentRow(-1);
	}
	m_size->setText("");
	if (m_rpe->m_textProp->m_fontSz>0){
		int pos = m_listFntSizes.indexOf(m_rpe->m_textProp->m_fontSz);
		if (pos != -1){
			m_sizeList->setCurrentRow(pos);
		}
		pos = m_rpe->m_textProp->m_fontSz;
		str = QString("%1").arg(pos);
		m_size->setText(str);
	}
	setupCheckState(m_rpe->m_textProp->m_fontI,m_chbItalic);
	setupCheckState(m_rpe->m_textProp->m_fontB,m_chbBold);
	setupCheckState(m_rpe->m_textProp->m_fontU,m_chbUnderline);
}

int getFromCheck(const QCheckBox* chb)
{
	Qt::CheckState state;
	int retVal = -1;
	state = chb->checkState();
	switch (state){
		case Qt::PartiallyChecked: 	{retVal = -1; break;}
		case Qt::Unchecked: 		{retVal = 0; break;}
		case Qt::Checked: 			{retVal = 1; break;}
		default: 					{break;}
	}
	return retVal;
}

void uoTextFontPropTab::applyResult()
{
	if (!m_rpe->m_textPropCopy)
		return;
	uoReportDoc* doc = m_rpe->getDoc();
	if (!doc)
		return;
	QString str = m_fontName->text();
	if (m_listFntNames.contains(str) && !str.isEmpty()) {
		///название шрифта валидно...
		m_rpe->m_textPropCopy->m_fontId = doc->addFont(str);
	}
	str = m_size->text();
	if (m_listFntSizes.contains(str.toInt())){
		m_rpe->m_textPropCopy->m_fontSz	= str.toInt();
	}
	m_rpe->m_textPropCopy->m_fontB = getFromCheck(m_chbBold);
	m_rpe->m_textPropCopy->m_fontI = getFromCheck(m_chbItalic);
	m_rpe->m_textPropCopy->m_fontU = getFromCheck(m_chbUnderline);
}


/// Перехватим некоторые эвенты для нормальной реализации диалога.
bool uoTextFontPropTab::eventFilter(QObject* pObj, QEvent* pEvent)
{
	if (pEvent->type() == QEvent::KeyPress) {
        QKeyEvent * k = (QKeyEvent *)pEvent;
        int key = k->key();

        if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_PageUp || key == Qt::Key_PageDown)
        {
			if (pObj == m_fontName){
	            int ci = m_listFonts->currentRow();
				(void)QApplication::sendEvent(m_listFonts, k);
	            if (ci != m_listFonts->currentRow())
					m_fontName->selectAll();
				return true;
			} else if (pObj == m_size) {
	            int ci = m_sizeList->currentRow();
				(void)QApplication::sendEvent(m_sizeList, k);
	            if (ci != m_sizeList->currentRow())
					m_size->selectAll();
				return true;
			}

        } else if (key == Qt::Key_Tab) {
			if (pObj == m_fontName){
				m_size->setFocus();
				return true;
			} else if (pObj == m_size) {
				Qt::KeyboardModifiers  kbrdMod = qApp->keyboardModifiers();
				if (kbrdMod & Qt::ShiftModifier){
					m_fontName->setFocus();
				} else {
					m_chbItalic->setFocus();
				}
				return true;
			}
        }
	}
	return QWidget::eventFilter(pObj, pEvent);
}


int uoTextFontPropTab::findFontItem(QString& fontName)
{
	///\todo доделать поиск наиболее подходящего итема....

	int retVal = -1;
	if (fontName.isEmpty())
		return retVal;
	QString fontNameLC = fontName.toLower();
	QListWidgetItem* item = NULL;

	int row_max_like = -1, char_max_like = -1, comp_rez = 0;
	QString str1;

	int len = fontName.length();

	for (int i = 1; i<=m_listFonts->count();i++){
		item = m_listFonts->item(i);
		if (item){
			str1 = item->text();
			comp_rez = str1.compare(fontName, Qt::CaseInsensitive);
			str1 = str1.toLower();
			if (comp_rez == 0)	{
				return i;
			} else {
				for (int y = 0; y< qMin(len,str1.length()); y++)
				{
					if (str1[y] != fontNameLC[y])
						break;
					if (char_max_like<y){
						char_max_like = y;
						row_max_like = i;
					}
				}
			}
		}
	}
	if (row_max_like != 0)
		retVal = row_max_like;
	return retVal;
}


void uoTextFontPropTab::onFontNameEditFinish()
{
	QString str1 = m_fontName->text();
	if (!m_listFntNames.contains(str1))
	{
		QListWidgetItem* item = m_listFonts->currentItem();
		if (item){
			m_fontName->setText(item->text());
		}
		m_size->setFocus(Qt::TabFocusReason);

	}
	fillFontSizeList();
}

void uoTextFontPropTab::fillFontSizeList()
{
	QString fName = m_fontName->text();
	if (fName.isEmpty())
		return;
	if (!m_listFntNames.contains(fName))
		return;

	int i = 0;
	int current = -1;
	int size = m_size->text().toInt();
	if (size == 0)
		size = 8;

//	qDebug()<<" font name: " << fName;

	QList<int> sizes = m_rpe->m_fontBD.pointSizes(fName);
	m_sizeList->blockSignals(true);
	m_sizeList->clear();

	for(QList<int>::const_iterator it = sizes.constBegin() ; it != sizes.constEnd(); ++it) {
		m_sizeList->addItem(QString::number(*it));
		if (current == -1 && *it >= size)
			current = i;
		++i;
	}
	m_sizeList->blockSignals(false);

	if (current != -1)
		m_sizeList->setCurrentRow(current);
}

void uoTextFontPropTab::onFontNameChange(const QString& str)
{
	//qDebug()<<str;
	bool needFind = true;
	QString str1, str2 = str;

	QListWidgetItem* item = m_listFonts->currentItem();
	if (item){
		str1 = item->text();
		if (str1.compare(str2)==0)
			needFind = false;
	}
	if (needFind)
	{
		m_listFonts->blockSignals(true);
		int row = findFontItem(str2);
		if (row != -1){
			m_listFonts->setCurrentRow(row);
		}
		m_listFonts->blockSignals(false);
	}
	fillFontSizeList();
}

uoBorderPropTab::uoBorderPropTab(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
//	QComboBox* 		m_leftCbSizer;
//	QComboBox* 		m_topCbSizer;
//	QComboBox* 		m_rightCbSizer;
//	QComboBox* 		m_bottomCbSizer;

	QStringList sizes;
	sizes<<""<<"1"<<"2"<<"3"<<"4"<<"5"<<"6";
	bool needSz = false;

	m_leftBFCh = new uoFrameChooser(m_leftBorderFrame);
	if (needSz) {
	m_leftCbSizer = new QComboBox(m_leftBorderFrame);
	m_leftCbSizer->addItems(sizes);
	}
	QHBoxLayout* layot_1 = new QHBoxLayout(m_leftBorderFrame);
	layot_1->addWidget(m_leftBFCh);
	if (needSz) layot_1->addWidget(m_leftCbSizer);
	layot_1->setSpacing(0);    layot_1->setMargin(0);

	m_topBFCh = new uoFrameChooser(m_topBorderFrame);
	if (needSz) {
	m_topCbSizer = new QComboBox(m_topBorderFrame);
	m_topCbSizer->addItems(sizes); }

	QHBoxLayout* layot_2 = new QHBoxLayout(m_topBorderFrame);
	layot_2->addWidget(m_topBFCh);
	if (needSz) layot_2->addWidget(m_topCbSizer);
	layot_2->setSpacing(0);    layot_2->setMargin(0);

	m_rightBFCh = new uoFrameChooser(m_rightBorderFrame);
	if (needSz) {
	m_rightCbSizer = new QComboBox(m_rightBorderFrame);
	m_rightCbSizer->addItems(sizes);
	}
	QHBoxLayout* layot_3 = new QHBoxLayout(m_rightBorderFrame);
	layot_3->addWidget(m_rightBFCh);
	if (needSz) layot_3->addWidget(m_rightCbSizer);
	layot_3->setSpacing(0);    layot_3->setMargin(0);

	m_bottomBFCh = new uoFrameChooser(m_bottomBorderFrame);
	if (needSz) {
	m_bottomCbSizer = new QComboBox(m_bottomBorderFrame);
	m_bottomCbSizer->addItems(sizes);
	}
	QHBoxLayout* layot_4 = new QHBoxLayout(m_bottomBorderFrame);
	layot_4->addWidget(m_bottomBFCh);
	if (needSz) layot_4->addWidget(m_bottomCbSizer);
	layot_4->setSpacing(0);    layot_4->setMargin(0);

	m_outlineBFCh = new uoFrameChooser(m_outlineBorderFrame);
	QHBoxLayout* layot_5 = new QHBoxLayout(m_outlineBorderFrame);layot_5->addWidget(m_outlineBFCh); layot_5->setSpacing(0);    layot_5->setMargin(0);
	hide();

}
uoBorderPropTab::~uoBorderPropTab()
{}
void uoBorderPropTab::initFromRPE()
{
	m_leftBFCh->setBorderType(m_rpe->m_borderProp->m_bordType[0]);
	m_topBFCh->setBorderType(m_rpe->m_borderProp->m_bordType[1]);
	m_rightBFCh->setBorderType(m_rpe->m_borderProp->m_bordType[2]);
	m_bottomBFCh->setBorderType(m_rpe->m_borderProp->m_bordType[3]);

	m_outlineBFCh->setBorderType(uoCBT_Unknown);
	if (m_rpe->m_sellectonType == uoRst_Unknown || m_rpe->m_sellectonType == uoRst_Cell)
	{
		uoCellBorderType cbt = m_rpe->m_borderProp->m_bordType[0];

		for (int i =1; i<4; i++){
			if (cbt != m_rpe->m_borderProp->m_bordType[i]){
				cbt = uoCBT_Unknown;
				break;
			}
		}
		m_outlineBFCh->setBorderType(cbt);

	}

}
void uoBorderPropTab::applyResult()
{
	if (!m_rpe)
		return;
	if (!m_rpe->m_borderPropCopy)
		return;
	uoCellBorderType bt = uoCBT_Unknown;
	for (int i=0; i<4; i++)
	{
		switch (i) {
			case 0: { bt = m_leftBFCh->borderType(); break;}
			case 1: { bt = m_topBFCh->borderType(); break;}
			case 2: { bt = m_rightBFCh->borderType(); break;}
			case 3: { bt = m_bottomBFCh->borderType(); break;}
			default:{break;}
		}
		if (m_rpe->m_borderPropCopy->m_bordType[i] != bt)
			m_rpe->m_borderPropCopy->m_bordType[i] = bt;
	}
}



uorPropDlg::uorPropDlg(QWidget *parent, uoReportPropEditor* pe)
	:QWidget(parent), m_pe(pe)
{
	setupUi(this);
	hide();
//	m_pe = NULL;
	setWindowFlags(Qt::Tool);
	connect(m_btnOK, SIGNAL(clicked(bool)), this, SLOT(onApply()));
	connect(m_btnCancel, SIGNAL(clicked(bool)), this, SLOT(onCancel()));
	connect(m_btmUpdate, SIGNAL(clicked(bool)), this, SLOT(onApplyWithoutHide()));

	m_tabTxt = new uoTextPropTab;
	if (m_tabTxt){
		m_tabTxt->setTypeTab(uorPropTTab_Text);
		m_tabTxt->setPropEditor(m_pe);
	}
	m_tabLayot = new uoTextLayotTab;
	if (m_tabLayot){
		m_tabLayot->setTypeTab(uorPropTTab_TextLayot);
		m_tabLayot->setPropEditor(m_pe);
	}
	m_tabFont = new uoTextFontPropTab;
	if (m_tabFont){
		m_tabFont->setTypeTab(uorPropTTab_TextFont);
		m_tabFont->setPropEditor(m_pe);
	}
	m_tabBorder = new uoBorderPropTab;
	if (m_tabBorder) {
		m_tabBorder->setTypeTab(uorPropTTab_Border);
		m_tabBorder->setPropEditor(m_pe);
	}

}

uorPropDlg::~uorPropDlg()
{
	propFrame->clear();
	m_tabsUsing.clear();
	delete	m_tabTxt;
	delete	m_tabLayot;
	delete	m_tabFont;
	delete	m_tabBorder;

}


/// Сливаем проперти из гуя в структуру.
bool uorPropDlg::applyResult()
{
	uorPropertyTabType tabType;
	for (int i = 0; i < m_tabsUsing.size(); ++i) {
		tabType = m_tabsUsing.at(i);
		switch(tabType)
		{
			case uorPropTTab_Text:		{	m_tabTxt->applyResult();	break;	}
			case uorPropTTab_TextLayot:	{	m_tabLayot->applyResult();	break;	}
			case uorPropTTab_TextFont:	{	m_tabFont->applyResult();	break;	}
			case uorPropTTab_Border:	{	m_tabBorder->applyResult();	break;	}
			default:{
				break;
			}
		}
	}
	return true;
}

void uorPropDlg::moveEvent ( QMoveEvent * event )
{
	QWidget::moveEvent ( event );
	savePosition();
}

void uorPropDlg::resizeEvent ( QResizeEvent * event )
{
	QWidget::resizeEvent( event );
	savePosition();
}
void uorPropDlg::savePosition()
{
	if (m_pe){
		// сохранил позицию до следующего показа.
		m_pe->m_lastGeometry = geometry();
	}
}

void uorPropDlg::keyPressEvent ( QKeyEvent * event )
{
	int key = event->key();
	switch (key)
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			event->accept();
			onApply();
			break;
		}
		case Qt::Key_Escape:
		{
			event->accept();
			onCancel();
			break;
		}
		default:{
			QWidget::keyPressEvent ( event );
			break;
		}
	}
}

void uorPropDlg::addTab(QWidget* tab, const QString& label, uorPropertyTabType tabType)
{
	if (propFrame && tab){
		propFrame->addTab(tab, label);
		if (!m_tabsUsing.contains(tabType))
			m_tabsUsing.append(tabType);

	}
}
void uorPropDlg::clearTabs()
{
	propFrame->clear();
	m_tabsUsing.clear();
}

/// Применить свойстра и спрятать панель
void uorPropDlg::onApply()
{
	m_pe->m_canselApply = false;
	applyResult();
	m_pe->hideProperty(true);
}
/// Применить свойстра и НО НЕ спрятать панель
void uorPropDlg::onApplyWithoutHide()
{
	applyResult();
	m_pe->m_canselApply = false;
	m_pe->applyProps();
}

/// Спрятать панель НЕ применять свойстра
void uorPropDlg::onCancel()
{
	m_pe->m_canselApply = true;
	m_pe->hideProperty(false);
}

bool uorPropDlg::initFromCtrl(uoReportCtrl* pCtrl)
{
	bool rez = false;
	if (pCtrl){
		bool isVis = isVisible();
		if (!isVis)
			clearTabs();
		uoReportDoc* doc = pCtrl->getDoc();

		if (doc) {
			rez = true;

			bool permTxtPrp = true; // свойства текста: тип и поведение

			if (permTxtPrp) {
				if (m_tabTxt){
					m_tabTxt->initFromRPE();
					if (!isVis)
						addTab(m_tabTxt, QString::fromUtf8("Текст"), m_tabTxt->m_typeTab);
				}
				if (m_tabLayot){
					m_tabLayot->initFromRPE();
					if (!isVis)
						addTab(m_tabLayot, QString::fromUtf8("Выравнивание"), m_tabLayot->m_typeTab);

				}
			}
			if (m_tabBorder){
				m_tabBorder->initFromRPE();
				if (!isVis)
					addTab(m_tabBorder, QString::fromUtf8("Рамка"), m_tabBorder->m_typeTab);

			}
			if(m_tabFont){
				m_tabFont->initFromRPE();
				if (!isVis)
					addTab(m_tabFont, QString::fromUtf8("Шрифт"), m_tabFont->m_typeTab);
			}
		}
	}
	return rez;

}


uoReportPropEditor::uoReportPropEditor(QObject* pObj)
	:QObject(pObj)
{
	m_propDlg 		= NULL;
	m_reportCtrl 	= NULL;
	m_writingSystem = QFontDatabase::Cyrillic;
	m_textProp 		= new uorTextDecor;
	m_textPropCopy 	= new uorTextDecor;
	m_borderProp 	= new uorBorderPropBase;
	m_borderPropCopy= new uorBorderPropBase;
	m_sellectonType = uoRst_Unknown;
	m_canselApply	= false;
	m_outlineBT 	= uoCBT_Unknown;
	m_outlineBTCopy = uoCBT_Unknown;


}

uoReportPropEditor::~uoReportPropEditor()
{
	delete m_textProp;
	delete m_textPropCopy;
	delete m_borderProp;
	delete m_borderPropCopy;
}

/// Вычислим те свойства которые необъодимо изменить и поместим их в m_textPropCopy
bool uoReportPropEditor::applyResult()
{
	m_textPropCopy->copyFrom(m_textProp);
	m_borderPropCopy->copyFrom(m_borderProp);
	return m_propDlg->applyResult();
}

/// Проперти изменились?
bool uoReportPropEditor::isChangedProperty()
{
	bool retVal = false;
	applyResult();
	retVal = m_textProp->isEqual(*m_textPropCopy);
	return !retVal;


}

bool uoReportPropEditor::initFromCtrl(uoReportCtrl* pCtrl)
{
	bool rez = false;
	if (m_propDlg && pCtrl){
		m_textProp->resetItem();
		m_borderProp->resetItem();
		pCtrl->populatePropEditor(this); // Заполнили редактор, теперь надо заполнить диалоги редактора
		rez = m_propDlg->initFromCtrl(pCtrl);
		m_textPropCopy->copyFrom(m_textProp);
		m_borderPropCopy->copyFrom(m_borderProp);

		m_canselApply = false;
	}
	return rez;
}

void uoReportPropEditor::hideProperty(const bool& save)
{
	if (m_propDlg){
		m_canselApply = !save;
		if (save){
			m_propDlg->applyResult();
			applyProps();
		}
		m_propDlg->hide();
	}
}

/// Применяем свойства к репорту.
bool uoReportPropEditor::applyProps()
{
	if (m_reportCtrl){
		m_reportCtrl->propertyEditorApply();
		return true;
	}
	return false;
}

bool uoReportPropEditor::editorIsVisible()
{
	bool retVal = false;
	if (m_propDlg){
		retVal = m_propDlg->isVisible();
	}
	return retVal;
}

uoReportDoc* uoReportPropEditor::getDoc()
{
	if (!m_reportCtrl)
		return NULL;
	return m_reportCtrl->getDoc();
}

bool uoReportPropEditor::showProperty(uoReportCtrl* pCtrl, bool forseActivate)
{
	bool rez = false;
	bool firstShow = false;
	if (!m_propDlg){
		firstShow = true;
		m_propDlg = new uorPropDlg(pCtrl, this);
		if (m_propDlg){
			m_propDlg->setPropEditor(this);
		}
	}
	if (!m_propDlg)
		return false;
	m_reportCtrl = pCtrl;
	rez = initFromCtrl(pCtrl);
	if (rez){
//		bool active = m_propDlg->isActiveWindow();
		if (!m_propDlg->isVisible()) {
			if (!m_lastGeometry.isValid()){
				m_lastGeometry = m_propDlg->geometry();
				m_lastGeometry.moveCenter(qApp->desktop()->availableGeometry().center());
			}
			m_propDlg->setGeometry(m_lastGeometry);
			m_propDlg->show();
			m_propDlg->activateWindow();
		} else {
			if (forseActivate)
				m_propDlg->activateWindow();

		}
	}
	return rez;
}

} //namespace uoReport
