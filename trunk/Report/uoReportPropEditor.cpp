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
	:QWidget(parent){
	setupUi(this);
	hide();
}

uoTextPropTab::~uoTextPropTab()
{}

void uoTextPropTab::init()
{
	this->m_TextType->addItem(QString::fromUtf8(""),uoCTT_Unknown);
	this->m_TextType->addItem(QString::fromUtf8("Text"),uoCTT_Text);
	this->m_TextType->addItem(QString::fromUtf8("Expr"),uoCTT_Expr);
	this->m_TextType->addItem(QString::fromUtf8("Templ"),uoCTT_Templ);

	this->m_TextBehavior->addItem(QString::fromUtf8(""),uoCTB_Unknown);
	this->m_TextBehavior->addItem(QString::fromUtf8("Auto"),uoCTB_Auto);
	this->m_TextBehavior->addItem(QString::fromUtf8("Cut"),uoCTB_Cut);
	this->m_TextBehavior->addItem(QString::fromUtf8("Obstruct"),uoCTB_Obstruct);
	this->m_TextBehavior->addItem(QString::fromUtf8("Transfer"),uoCTB_Transfer);
}

uoTextLayotTab::uoTextLayotTab(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	hide();
}
uoTextLayotTab::~uoTextLayotTab()
{}

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

	qDebug()<<" font name: " << fName;

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




uorPropDlg::uorPropDlg(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	hide();
	setWindowFlags(Qt::Tool);
}

uorPropDlg::~uorPropDlg()
{
	propFrame->clear();
	QWidget* wi = NULL;
	QMap<uorPropertyTabType, QWidget*>::iterator i = m_tabs.begin();
	while(i != m_tabs.end()){
		wi = *i;
		delete wi;
		i++;
	}
}

void uorPropDlg::initFontTab(uoTextFontPropTab* tab)
{
	if (!tab)
		return;
	// tab->m_listFonts >> 	QListWidget
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


QWidget* uorPropDlg::getTab(uorPropertyTabType tabType)
{
	QWidget* wi = NULL;
	if (!m_tabs.contains(tabType))
	{
		/**		\todo сделано немного поблядски, надобы табсам унаследоваться от виджета а этот свитч похоронить..		*/
		switch(tabType)
		{
			case uorPropTab_Text:
			{

				uoTextPropTab* tab = new uoTextPropTab;
				if (tab){
					tab->setPropEditor(m_pe);
					tab->setTypeTab(uorPropTab_Text);
					wi = tab;
				}
				break;
			}
			case uorPropTab_TextLayot:
			{
				uoTextLayotTab* tab = new uoTextLayotTab;
				if (tab){
					tab->setPropEditor(m_pe);
					tab->setTypeTab(uorPropTab_TextLayot);
					wi = tab;
				}
				break;
			}

			case uorPropTab_TextFont:
			{
				uoTextFontPropTab* tab = new uoTextFontPropTab;
				if (tab){
					initFontTab(tab);
					tab->setPropEditor(m_pe);
					tab->setTypeTab(uorPropTab_TextFont);

					wi = tab;
				}
				break;
			}


			case uorPropTab_Unknown:
			default:{
				break;
			}
		}
		if (wi){
			m_tabs.insert(tabType,wi);
		}
	} else {
		wi = m_tabs[tabType];
	}

	return wi;
}
void uorPropDlg::addTab(QWidget* tab, const QString& label)
{
	if (propFrame && tab){
		propFrame->addTab(tab, label);
	}
}
void uorPropDlg::clearTabs()
{
	propFrame->clear();
}


uoReportPropEditor::uoReportPropEditor(QObject* pObj)
	:QObject(pObj)
{
	m_propDlg 		= NULL;
	m_reportCtrl 	= NULL;
	m_writingSystem = QFontDatabase::Cyrillic;
	m_textProp 		= new uorTextDecor;
	m_borderProp 	= new uorBorderPropBase;
	m_sellectonType = uoRst_Unknown;

}

uoReportPropEditor::~uoReportPropEditor()
{
	delete m_textProp;
	delete m_borderProp;
}



bool uoReportPropEditor::initFromCtrl(uoReportCtrl* pCtrl)
{
	bool rez = false;
	if (m_propDlg && pCtrl){
		m_propDlg->clearTabs();
		uoReportDoc* doc = pCtrl->getDoc();

		if (doc) {
			rez = true;

			/* наверное пусть сам контрол соберет совокупную информацию.
			Там сложности с выделениями и т.п. вещами. а я тут просто проанализирую,
			что он там впихнул мне в m_textProp и m_borderProp и проинициализирую их диалоги.*/

			uoReportSelection* selMengr = pCtrl->getSelection();
			uoRptSelectionType selType = selMengr->getSelectionType();
			// permissible - it is permissible ( это разрешается)
			bool permTxt = false; // сам текст и расшифровка
			bool permTxtPrp = true; // свойства текста: тип и поведение
			switch(selType){
				case uoRst_Unknown:{
					permTxt = true;
					break;
				}
				case uoRst_Document:
				case uoRst_Column:
				case uoRst_Columns:
				case uoRst_Row:
				case uoRst_Rows:
				case uoRst_Cell:
				case uoRst_Cells:
				case uoRst_Mixed:{

					break;
				}
			}
			const QPoint pt	= pCtrl->getCurentCell();
			m_textProp->resetItem();
			pCtrl->populatePropEditor(this);

			uoCell* cell = doc->getCell(pt.y(),pt.x(),false);
			if (permTxtPrp) {
				uoTextPropTab* tabTxt = (uoTextPropTab*)m_propDlg->getTab(uorPropTab_Text);
				uorTextDecor* prop = NULL;
				if (cell){
					prop = cell->getTextProp(doc, true);
				}
				if (!prop){
					prop = doc->getDefaultTextProp();
				}
				if (tabTxt){
					m_propDlg->addTab(tabTxt, QString::fromUtf8("Текст"));
					if (prop){
						tabTxt->m_textCell->setEnabled(permTxt);
						tabTxt->m_textDecoding->setEnabled(permTxt);
						if (permTxt && cell){
							tabTxt->m_textCell->setPlainText(cell->m_text);
							tabTxt->m_textDecoding->setText(cell->m_textDecode);
						}
						tabTxt->m_TextBehavior->setCurrentIndex(prop->m_TextBehavior);
						tabTxt->m_TextType->setCurrentIndex(prop->_textType);
					}
				}
				uoTextLayotTab* tabLayot = (uoTextLayotTab*)m_propDlg->getTab(uorPropTab_TextLayot);
				if (tabLayot){
					m_propDlg->addTab(tabLayot, QString::fromUtf8("Выравнивание"));
					tabLayot->m_TopVTA->setChecked(false);
					tabLayot->m_CenterVTA->setChecked(false);
					tabLayot->m_BottomVTA->setChecked(false);

					tabLayot->m_LeftHTA->setChecked(false);
					tabLayot->m_CenterHTA->setChecked(false);
					tabLayot->m_RightHTA->setChecked(false);
					if (prop){
						switch (prop->m_horTAlignment){
						case uoHA_Left: 	{tabLayot->m_LeftHTA->setChecked(true); break;}
						case uoHA_Center: 	{tabLayot->m_CenterHTA->setChecked(true); break;}
						case uoHA_Right: 	{tabLayot->m_RightHTA->setChecked(true); break;}
						default:
							break;
						}
						switch (prop->m_vertTAlignment){
						case uoVA_Top: 		{tabLayot->m_TopVTA->setChecked(true); break;}
						case uoVA_Center: 	{tabLayot->m_CenterVTA->setChecked(true); break;}
						case uoVA_Bottom: 	{tabLayot->m_BottomVTA->setChecked(true); break;}
						default:
							break;
						}
					}
				}
			}
			uoTextFontPropTab* tabFont = (uoTextFontPropTab*)m_propDlg->getTab(uorPropTab_TextFont);
			// QComboBox
			if (tabFont){
				m_propDlg->addTab(tabFont, QString::fromUtf8("Шрифт"));
			}


		}
	}
	return rez;
}

void uoReportPropEditor::hidePriperty(const bool& save)
{
	if (m_propDlg){
		m_propDlg->hide();
	}
}

bool uoReportPropEditor::editorIsVisible()
{
	bool retVal = false;
	if (m_propDlg){
		retVal = m_propDlg->isVisible();
	}
	return retVal;
}

bool uoReportPropEditor::showPriperty(uoReportCtrl* pCtrl)
{
	bool rez = false;
	bool firstShow = false;
	if (!m_propDlg){
		firstShow = true;
		m_propDlg = new uorPropDlg(pCtrl);
		if (m_propDlg){
			m_propDlg->setPropEditor(this);
		}
	}
	if (!m_propDlg)
		return false;
	rez = initFromCtrl(pCtrl);
	if (rez){
		if (!m_propDlg->isVisible()) {
			if (!m_lastGeometry.isValid()){
				m_lastGeometry = m_propDlg->geometry();
				m_lastGeometry.moveCenter(qApp->desktop()->availableGeometry().center());
			}
			m_propDlg->setGeometry(m_lastGeometry);
//			m_propDlg->move( QRect( QPoint(), ( QApplication::desktop()->size() - m_propDlg->size() ) / 2 ).bottomLeft() );
			m_propDlg->show();
			m_propDlg->activateWindow();
		}
	}
	return rez;
}


} //namespace uoReport
