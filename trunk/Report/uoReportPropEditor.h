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
#include "uoReportDoc.h"
#include "uoReportCtrl.h"
#include "ui_uorPropDlg.h"
#include "ui_uorTextProp.h"
#include "ui_uorTextLayotProp.h"
#include "ui_uorTextFontProp.h"
class QStandardItemModel;
struct uoCell;
struct QDesktopWidget;



namespace uoReport {

// Декоратор
class uorCommTab{
	public:
		inline void setPropEditor(uoReportPropEditor* pe){m_rpe = pe; init();};
		inline void setTypeTab(const uorPropertyTabType&	typeTab){m_typeTab = typeTab;};
		virtual void init(){};
		uoReportPropEditor* m_rpe;
		uorPropertyTabType	m_typeTab;
};

class uoTextPropTab : public QWidget, public Ui::uorTextProp, public uorCommTab
{
	public:
		uoTextPropTab(QWidget *parent = 0);
		virtual ~uoTextPropTab();
		virtual void init();
};
class uoTextLayotTab : public QWidget, public Ui::uorTextLayotProp, public uorCommTab
{
	public:
		uoTextLayotTab(QWidget *parent = 0);
		virtual ~uoTextLayotTab();
};

class uoTextFontPropTab : public QWidget, public Ui::uorTextFontProp, public uorCommTab
{
    Q_OBJECT
	public:
		uoTextFontPropTab(QWidget *parent = 0);
		virtual ~uoTextFontPropTab();
		virtual void init();
		void fillFontSizeList();
	public slots:
		void onFontNameChange(const QString& str);
		void onFontNameEditFinish();
	protected:
		int findFontItem(QString& fontName);
		QStringList m_listFntNames;
		QCompleter* m_cmpl;
		uoColorChooser* m_ColChooser;
	protected:
		virtual bool eventFilter(QObject* pObj, QEvent* pEvent);


};


class uorPropDlg : public QWidget, public Ui::uoPropDlg
{
	Q_OBJECT
	public:
		uorPropDlg(QWidget *parent = 0);
		virtual ~uorPropDlg();
		inline void setPropEditor(uoReportPropEditor* pe){m_pe = pe;};

		QWidget* getTab(uorPropertyTabType tabType);
		void addTab(QWidget* tab, const QString& label);
		void clearTabs();
		void initFontTab(uoTextFontPropTab* tab);
	protected:
		virtual void moveEvent ( QMoveEvent * event );
		virtual void resizeEvent ( QResizeEvent * event );
		void savePosition();

	private:
		QTabWidget* m_tabWidget;
		QMap<uorPropertyTabType, QWidget*> m_tabs;
		uoReportPropEditor* m_pe;

};

/**
	class uoReportPropEditor - объект, координирующий плавающую панетль свойств табличного редактора.
*/
class uoReportPropEditor : public QObject
{
	Q_OBJECT
	public:
		uoReportPropEditor(QObject* pObj = 0);
		virtual ~uoReportPropEditor();

		bool showPriperty(uoReportCtrl* pCtrl);
		bool applyProps()	{return true;}
		bool initFromCtrl(uoReportCtrl* pCtrl);
		void hidePriperty(const bool& save = false);
		bool editorIsVisible();

	private:
		uorPropDlg* 	m_propDlg;
		uoReportCtrl* 	m_reportCtrl;
	public:
		uorTextDecor* 	m_textProp; 	///< закешированное значения свойств.
		uorBorderPropBase* 	m_borderProp;	///< закешированное значения свойств.
		QFontDatabase::WritingSystem m_writingSystem;
		QFontDatabase 	m_fontBD;

		QRect 			m_lastGeometry; ///< запоминаем последнее расположение палитры свойств.
		/* надо поделить на логические части контролы для редактирования.
		в разных режимах могут быть доступны не все свойства сразу.
		например при выделении колонок текст не отредактируешь.
		В принцыпе пока можно опереться на uoRptSelectionType selType*/
		uoRptSelectionType m_sellectonType;
};
} //namespace uoReport
#endif // UOREPORTPROPEDITOR_H
