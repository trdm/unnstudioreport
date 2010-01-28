/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTPROPEDITOR_H
#define UOREPORTPROPEDITOR_H

#include "uoReport.h"
#include <QFontDatabase>
#include <QCompleter>
#include <QMap>
#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QTabWidget>
#include <QString>
#include "uoColorChooser.h"
#include "uoFrameChooser.h"
#include "uoReportDoc.h"
#include "uoReportCtrl.h"
#include "ui_uorPropDlg.h"
#include "ui_uorTextProp.h"
#include "ui_uorTextLayotProp.h"
#include "ui_uorTextFontProp.h"
#include "ui_uorBorderProp.h"
class QStandardItemModel;
struct uoCell;
struct QDesktopWidget;



namespace uoReport {

/**
	\class uorCommTab - Декоратор для всех закладок в редакторе свойств.
	\brief Декоратор для всех закладок в редакторе свойств.
*/
class uorCommTab{
	public:
		inline void setPropEditor(uoReportPropEditor* pe){m_rpe = pe; init();};
		inline void setTypeTab(const uorPropertyTabType&	typeTab){m_typeTab = typeTab;};
		virtual void init(){};
		virtual void initFromRPE(){};
		virtual void applyResult(){};
		uoReportPropEditor* m_rpe;
		uorPropertyTabType	m_typeTab;
};

/**
	\class uoTextPropTab - Виджет для закладки редактирования текста и его общих свойств.
	\brief Виджет для закладки редактирования текста и его общих свойств.
*/
class uoTextPropTab : public QWidget, public Ui::uorTextProp, public uorCommTab
{
	public:
		uoTextPropTab(QWidget *parent = 0);
		virtual ~uoTextPropTab();
		virtual void init();
		virtual void initFromRPE();
		virtual void applyResult();
};

/**
	\class uoTextLayotTab - Виджет для закладки редактирования опций выравнивания текста.
	\brief Виджет для закладки редактирования опций выравнивания текста.
*/
class uoTextLayotTab : public QWidget, public Ui::uorTextLayotProp, public uorCommTab
{
	public:
		uoTextLayotTab(QWidget *parent = 0);
		virtual ~uoTextLayotTab();
		virtual void initFromRPE();
		virtual void applyResult();
	private:
		QButtonGroup* m_grVA;
		QButtonGroup* m_grHA;

};


/**
	\class uoBorderPropTab - Виджет для закладки редактирования опций бордюров ячеек.
	\brief Виджет для закладки редактирования опций бордюров ячеек.
*/
class uoBorderPropTab : public QWidget, public Ui::uorBorderProp, public uorCommTab
{
    Q_OBJECT
	public:
		uoBorderPropTab(QWidget *parent = 0);
		virtual ~uoBorderPropTab();
		virtual void initFromRPE();
		virtual void applyResult();
	private:
		uoFrameChooser* m_leftBFCh;
		uoFrameChooser* m_topBFCh;
		uoFrameChooser* m_rightBFCh;
		uoFrameChooser* m_bottomBFCh;
		uoFrameChooser* m_outlineBFCh;

		QComboBox* 		m_leftCbSizer;
		QComboBox* 		m_topCbSizer;
		QComboBox* 		m_rightCbSizer;
		QComboBox* 		m_bottomCbSizer;
};

/**
	\class uoTextFontPropTab - Виджет для закладки редактирования опций шрифта текста.
	\brief Виджет для закладки редактирования опций шрифта текста.
*/
class uoTextFontPropTab : public QWidget, public Ui::uorTextFontProp, public uorCommTab
{
    Q_OBJECT
	public:
		uoTextFontPropTab(QWidget *parent = 0);
		virtual ~uoTextFontPropTab();
		virtual void init();
		virtual void initFromRPE();
		virtual void applyResult();
		void fillFontSizeList();
	public slots:
		void onFontNameChange(const QString& str);
		void onFontNameEditFinish();
	protected:
		int findFontItem(QString& fontName);
		QStringList m_listFntNames;
		QList<int> 	m_listFntSizes;
		QCompleter* m_cmpl;
		uoColorChooser* m_ColChooser;
	protected:
		virtual bool eventFilter(QObject* pObj, QEvent* pEvent);


};


/**
	\class uorPropDlg - Собственно плавающая панель, контейнер для остальных закладок.
	\brief Собственно плавающая панель, контейнер для остальных закладок.
*/
class uorPropDlg : public QWidget, public Ui::uoPropDlg
{
	Q_OBJECT
	public:
		typedef QMap<uorPropertyTabType, QWidget*>::iterator uoTabPropIter;
	public:
		uorPropDlg(QWidget *parent = 0, uoReportPropEditor* pe = 0);
		virtual ~uorPropDlg();
		inline void setPropEditor(uoReportPropEditor* pe){m_pe = pe;};

//		QWidget* getTab(uorPropertyTabType tabType);
		void addTab(QWidget* tab, const QString& label, uorPropertyTabType tabType);
		void clearTabs();
		bool applyResult();

		bool initFromCtrl(uoReportCtrl* pCtrl);

	public slots:
		void onApply();
		void onApplyWithoutHide();
		void onCancel();

	protected:
		virtual void moveEvent ( QMoveEvent * event );
		virtual void resizeEvent ( QResizeEvent * event );
		virtual void keyPressEvent ( QKeyEvent * event );
		void savePosition();

	private:
		QTabWidget* m_tabWidget;
		QList<uorPropertyTabType> m_tabsUsing;
		uoReportPropEditor* m_pe;

		uoTextPropTab* 		m_tabTxt;
		uoTextLayotTab* 	m_tabLayot;
		uoTextFontPropTab* 	m_tabFont;
		uoBorderPropTab*	m_tabBorder;
};

/**
	\class uoReportPropEditor - объект, координирующий плавающую панетль свойств табличного редактора.
	\brief объект, координирующий плавающую панетль свойств табличного редактора.

	Использование приема: m_textProp+m_textPropCopy
	Смысл: необходимо определить и размножить по документу только те свойства,
	которые пользователь изменил, а не все. использование копии позволяет
	определить путем мерджинга(сравнения) эти измененные свойства...

*/
class uoReportPropEditor : public QObject
{
	Q_OBJECT
	public:
		uoReportPropEditor(QObject* pObj = 0);
		virtual ~uoReportPropEditor();

		bool applyProps();
		bool initFromCtrl(uoReportCtrl* pCtrl);
		bool applyResult();
		bool isChangedProperty();

		bool showProperty(uoReportCtrl* pCtrl, bool forseActivate = false);
		void hideProperty(const bool& save = false);
		bool editorIsVisible();
		uoReportDoc* getDoc();

	private:
		uorPropDlg* 	m_propDlg;
		uoReportCtrl* 	m_reportCtrl;

	public:
		uorSelectionType m_sellectonType;
		bool 	m_canselApply;
		QString m_cellText;		///< текст ячейки 		(если m_sellectonType = uoRst_Unknown)
		QString m_cellDecode;   ///< расшифровка ячейки (если m_sellectonType = )

		uorTextDecor* 		m_textProp; 		///< значения свойств текста.
		uorTextDecor* 		m_textPropCopy; 	///< значения свойств текста полученное мержингом с данных m_textProp и визуального предстваления
		uorBorderPropBase* 	m_borderProp;		///< значения свойств бордюров.
		uorBorderPropBase* 	m_borderPropCopy;	///< значения свойств бордюров.
		uoCellBorderType 	m_outlineBT;
		uoCellBorderType 	m_outlineBTCopy;

		QFontDatabase::WritingSystem m_writingSystem;
		QFontDatabase 		m_fontBD;

		QRect 			m_lastGeometry; ///< запоминаем последнее расположение палитры свойств.
		/* надо поделить на логические части контролы для редактирования.
		в разных режимах могут быть доступны не все свойства сразу.
		например при выделении колонок текст не отредактируешь.
		В принцыпе пока можно опереться на uorSelectionType selType*/
};
} //namespace uoReport
#endif // UOREPORTPROPEDITOR_H
