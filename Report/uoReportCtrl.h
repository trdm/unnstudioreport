/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#ifndef UOREPTOPRULER_H
#define UOREPTOPRULER_H

#include <QtGlobal>
#include <QWidget>
#include <QTextEdit>
#include <QFrame>
#include <QRect>
#include <QRectF>
#include <QBrush>
#include <QPen>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QShortcut>
#include "uoReport.h"
#include "uoReportDocBody.h"
#include "uoReportViewIteract.h"
#include "uoReportSelection.h"
#include "uoReportPropEditor.h"
#include "uoReportCtrlItems.h"
#include "uorReportAreas.h"
#include "uoPainter.h"


class uoReportDoc;
class uoReportUndo;
class QInputDialog;
struct uoCell;
struct uoEnumeratedItem;
struct uorTextDecor;
struct uorBorderPropBase;
class uoReportCtrl;
class uoReportDrawHelper;
class uorPagePrintSetings;
class uorPageSetup;
class uorMimeData;


namespace uoReport {


/**
	\class uoReportCtrlMesFilter класс перехватчик некоторых событий.
	\brief класс перехватчик некоторых событий.

	Конкретно нужно что вот что:
	1. перехватить Ctrl+Enter и превратить его в Enter
	А Enter отловить и поместить отредактированный текст обрабно документ и
	закончить редактирование.
	2. При изменении текста посчитать его длинну/высоту и изменить поле редактирования
	так, что-бы текст влезал в него по ширине...
*/
class uoReportCtrlMesFilter : public QObject
{
    Q_OBJECT
	public:
		uoReportCtrlMesFilter(QObject* pObj = 0);
	protected:
		virtual bool eventFilter(QObject*, QEvent*);
	signals:
        void editComplete(bool accept); ///< сигнал о завершении обработки текста.
};

class uoReportCtrl;

/**
	\struct uorSelVisitorBase - Абстрактный базовый класс обходчик выдления.
	\brief Абстрактный базовый класс обходчик выдления.
*/

struct uorSelVisitorBase
{
	uorSelVisitorBase()
		:m_ctrl(0),m_type(uorSVT_Unknown),m_needCreate(false)
	{
		m_textDec_Selection.resetItem();
		m_borderProp_Selection.resetItem();
	}
	uoReportCtrl* m_ctrl;
	uorSelVisitorType m_type;
	virtual bool visit(uoCell* cell, bool first = false) = 0;
	bool m_needCreate;
	uorTextDecor 		m_textDec_Selection;
	uorBorderPropBase 	m_borderProp_Selection;	///< закешированное значения свойств.


};

/**
	\struct uorSelVisitorGetProps - обходчик+сборшик пропертей по выделениям
	\brief обходчик+сборшик пропертей по выделениям
*/
struct uorSelVisitorGetProps : public uorSelVisitorBase
{
	uorSelVisitorGetProps()
		:uorSelVisitorBase() {m_type = uorSVT_Getter;}
	virtual bool visit(uoCell* cell, bool first = false);
};

/**
	\struct uorSelVisitorGetProps - обходчик+сборшик пропертей по выделениям
	\brief обходчик+сборшик пропертей по выделениям
*/
struct uorSelVisitorSetProps : public uorSelVisitorBase
{
	uorSelVisitorSetProps()
		:uorSelVisitorBase() {m_type = uorSVT_Setter;	}
	virtual bool visit(uoCell* cell, bool first = false);
};


/**
	\class uoReportCtrl виджет, обслуживающий отрисовку отчета в режиме разработки или использования печатной формы.
	\brief инструмент для рендринга отчета, его корректировки.
*/
class uoReportCtrl : public QWidget
{
    Q_OBJECT
    friend class uoReportDoc;
    friend class uoReportViewIteract;
    friend class uorReportAreaBase;
    friend class uoReportDrawHelper;
    public:
        uoReportCtrl(QWidget *parent = 0);
        virtual ~uoReportCtrl();

		void setDoc(uoReportDoc* rptDoc);
        void clear();
        bool populateMenu(QMenu* targMenu);
	private:
		Q_DISABLE_COPY(uoReportCtrl)
    protected:

        void paintEvent(QPaintEvent *event);
        void contextMenuEvent(QContextMenuEvent *event);
		void showEvent( QShowEvent* event );
		void resizeEvent ( QResizeEvent * event );
		void keyPressEvent ( QKeyEvent * event );
		void wheelEvent ( QWheelEvent * event );
		void focusInEvent ( QFocusEvent * event );
		void focusOutEvent ( QFocusEvent * event );
        void mouseDoubleClickEvent( QMouseEvent * event );
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
		//-------------------------------------------------
		bool mousePressEventForRuler(QMouseEvent *event, bool isDoubleClick = false);
		bool mousePressEventForGroup(QMouseEvent *event, bool isDoubleClick = false);
		bool mousePressEventForSection(QMouseEvent *event, bool isDoubleClick = false);
		bool mousePressEventForDataArea(QMouseEvent *event, bool isDoubleClick = false);
		bool findScaleLocation(uorNumber posX, uorNumber posY, int &scaleNo, uoRptHeaderType rht);

		uorSparesType findPointLocation(uorNumber posX, uorNumber posY);

		uorBorderLocType  scaleLocationInBorder(uorNumber pos, uorRect rect, uoRptHeaderType rht);
		void keyPressEventMoveCursor ( QKeyEvent * event );
		void keyPressEventShortcut ( QKeyEvent * event );
		void updateThis();

		int m_freezUpdate;	/// заморозить посылку сообщений на перерисовку

		// Акселераторы для поиска запчасти под курсором.
		uorSparesType 	m_curMouseSparesType;
		int 			m_curMouseSparesNo;
		uoRptHeaderType m_curMouseSparesRht;
		uorRect			m_curMouseSparesRect;
		/**
			последняя позиция мышки в которой было иницировано действие нажатием левой клавишы.
			Например иницировано начало выделения или изменения размера ячейки.
			если пользователь тут же отпускает мышку, но рука дергается возникает нежелательный
			еффект перетаскивания. Это надо исключить. и исключим с пом. этой m_curMouseLastPos.
		*/
		QPoint			m_curMouseLastPos;
		QPoint			m_curMouseCurPos;
		void 			mouseSparesAcceleratorDrop();
		void 			mouseSparesAcceleratorSave(uorSparesType spar, int nom, uoRptHeaderType rht);

		QPoint 	m_curentCell; ///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		QRect 	m_curentCellRect; ///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		void	checkCurentCell(int& col, int& row, int& colsJn, int& rowsJn);
		void	setCurentCell(const int& colTmp, const int& rowTmp, bool ensureVisible = false);
		QRect 	getCellRect(const int& rowNo, const int& colNo, bool ifJoinGetFull = false);
		QPoint 	getCellFromPosition(const uorNumber& posY, const uorNumber& posX);
		bool 	curentCellVisible();

	public slots:
		void	onSetVScrolPos(int y);
		void	onSetHScrolPos(int x);
		void	onScrollActionV(int act);
		void	onScrollActionH(int act);
		void	doScrollAction(int act, uoRptHeaderType rht);
		void 	scrollView(int dx, int dy);

		// тут получаем комманды от меню
		void 	onCopy();
		void 	onPaste();
		void 	onCut();

	signals:
		void onColumnSizeChange(const int& nmColl, const uorNumber& nmLen);
		void onSelectionChange();
		void onCurentCellChange(const QPoint& oldPoint, const QPoint& newPoint);
		void onStateModeChange(const uoReportStateMode& oMode, const uoReportStateMode& nMode);

	public:
		//-------- draw section ----------------
		void drawDataArea(uoPainter& painter, uorReportAreaBase& drArea);
		void initDrawInstruments();
    protected:
		void drawWidget(uoPainter& painter);
		void drawHeaderControlContour(uoPainter& painter);
		void drawHeaderControlGroup(uoPainter& painter);
		void drawHeaderControl(uoPainter& painter);
		void drawDataAreaResizeRuler(uoPainter& painter);
		void drawCell(uoPainter& painter, uoCell* cell, uorRect& rectCell, uoReportDoc* doc, bool isSell);
		void drawSpliter(uoPainter& painter);
		void drawCurentCell(uoPainter& painter);
		void drawAfterDataErea(uoPainter& painter);

		bool getOffsetFromLastArea(uoRptHeaderType& rht, uorNumber& offset);
		uoReportDrawHelper* m_drawHelper;

		/**
			Используется для расчленения обрасти просмотра отчета на части/закрепления опредененны областей
			областей для удобства нафигации/просмотра.
			m_fixationPoint - точка фиксации.
			x - кол-во фиксированных строк
			y - кол-во фиксированных колонок
		*/
		uorFixationTypes m_fixationType;
		uorPoint	m_fixationPoint;
		int 	m_fixedRow;
		int 	m_fixedCol;
		uorReportViewArea m_areas[4];
		uorReportViewArea m_areaMain;		///<главная область отчета.
		uorReportViewArea m_areaMainCopy;	///<Копия области отчета для определения отрисовки.
		int  m_dirtyDrawErea;

		/* Данные для оптимизации расшифровки */
		uorSelectionType m_cashDrawSellType; //= m_selections->selectionType();
		long m_cashDrawMaVer; 	 //= m_areaMain.changesVer();
		long m_cashDrawDocVer; 	 //= m_doc.changesVer();
		void checkRedrawOption();


		uorNumber m_charWidthPlus; 	///< Опорная ширина символа "+" в текушем шрифте.
		uorNumber m_charHeightPlus; ///< Опорная высота символа "+" в текушем шрифте.

		uorNumber m_scaleFactor;	///< Положительный соэффициент масштаба виджета, если он > 0, тогда виджет крупнее, если меньше, виджет мельче.
		uorNumber m_scaleFactorO;	///< обратная величина фактора. для пересчетов смещений.

	private:
		void 		initControls(QWidget *parent);
		QScrollBar  *m_vScrollCtrl;
		QScrollBar  *m_hScrollCtrl;
		QWidget* 	m_cornerWidget; //, m_cornerWidget(parent)
		void 		recalcScrollBars();

		QTextEdit*	m_textEdit;
		uoReportCtrlMesFilter* m_messageFilter;
		bool doCellEditTextStart(const QString& str);
		void recalcTextEditRect(QRect& rect);
		bool modeTextEditing();

	private slots:
		void onCellEditTextEnd(bool accept);
		void cellTextChangedFromEdit();
		void onSelectonChange(const uorSelectionType& sModeOld, const uorSelectionType& sModeNew);


	public:
		inline
		uoReportDoc* getDoc() {return m_rptDoc;}

		bool saveDoc();
		bool saveDocAs();
		void optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler);
		uoReportSelection* getSelection(){ return m_selections;};
		const QPoint 	getCurentCell() const {return m_curentCell;}

		bool propertyEditorInit(uoReportPropEditor* pPropEditor);
		bool populatePropEditor(uoReportPropEditor* propEditor);

		bool propertyEditorApply();
		bool propertyEditorVisible();

		bool	doFixedView(int rows = 0, int cols = 0);

	protected:
		void 	recalcFixationPointStart();

		uorNumber 	getWidhtWidget(); 	///< Ширина с учетом масштаба
		uorNumber 	getHeightWidget();	///< Высота с учетом масштаба

		bool 	rowVisible(const int& nmRow) const;
		bool 	colVisible(const int& nmCol) const;

		void 	recalcHeadersRects();
		void 	recalcFixedAreasRects();
		void 	recalcGroupSectionRects(uoRptHeaderType rht = uorRhtUnknown);
		uorNumber 	getLengthOfScale(const uoRptHeaderType& rht, const int& start, const int& stop);
		int  	recalcVisibleScales(const uoRptHeaderType& rht);
		void 	calcGroupItemPosition(uoRptGroupItem* grItem, uoRptHeaderType rht);

		void 	dropGropItemToCache();
		uoRptGroupItem* 	getGropItemFromCache();
		uoRptSectionItem* 	getSectionItemFromCache();

	private:
		void updateImage();

		// В эти переменные собираем свойства выделнных элементов.
		uorTextDecor 		m_textDec_Selection;
		uorBorderPropBase 	m_borderProp_Selection;	///< закешированное значения свойств.

		void recalcSelectionProperty();
		void processSelection(uorSelVisitorBase* processor);
		void setStateMode(uoReportStateMode stMode);
		void clearSelectionsSection();


	private slots:
		void debugRects();

		void onSave();
		void onSaveAs();
		void onLoad();
		void onClear();

		void onGridShow();
		void onGridHide();

		void onInvisibleCharShow();
		void onInvisibleCharHide();

		void onFrameShow();
		void onFrameHide();

		void onRulerShow();
		void onRulerHide();

		void onSectionShow();
		void onSectionHide();

		void onGroupShow();
		void onGroupHide();

		void onOutToDebug();
		void onSetScaleFactor(const uorNumber sFactor);

		void onSectionInclude();
		void onSectionExclude();

		void onGroupInclude();
		void onGroupExclude();

		void onUndo();
		void onRedo();
		void onDataChange();

		void propertyEditorShow();
		void propertyEditorShowActivate();
		void propertyEditorHide();
		void onPropChange();

		void onRowColDelete();
		void onRowColAdd();
		void onRowAutoSize();
		void onRowColSetSize();

		void onShowPreview();
		void onShowPagesSetings();

		void onLoadTXT();
		void onCreateMatrix();
		void onOptionsShow();

		void onCellJoin();

	private:
		void onRowColGroupOperation(const uorRCGroupOperationType& opertn);
		void onRowColGroupOperation2(const uorRCGroupOperationType& opertn);
//		void onRowColGroupOperation2(const uorRCGroupOperationType& opertn, const QList<int>& psList, const uoRptHeaderType& psRht) const;


	private:
		void onSpanInclude(uoRptSpanType spType);
		void onSpanExclude(uoRptSpanType spType);

		void onAccessRowOrCol(const int& nom, const uoRptHeaderType& rht); ///< при доступе к строке или столбцу вьюва...
		void onAccessRowCol(int nmRow = 0, int nmCol = 0); ///< при доступе к строке или столбцу вьюва...
		void doChangeVirtualSize(uoRptHeaderType rht, int changeCnt); ///< обработать смену виртуального размера

	private:
		/// данные/ректы для областей....
		uorRect m_rectGroupRow;		///< Вертикальные группировки
		uorRect m_rectGroupCol;		///< Горизонтальные группировки
		uorRect m_rectSectionRow;		///< Вертикальные секции
		uorRect m_rectSectionCol;		///< Горизонтальные секции
		uorRect m_rectRulerRow;		///< Вертикальная линейка
		uorRect m_rectRulerCol;		///< Горизонтальная линейка
		uorRect m_rectRuleCorner;	///< Верхний корнер-виджет слева от горизонтальной и сверху от вертикальной линейки
		uorRect m_rectAll;			///< Полный регион
		uorRect m_rectDataRegion;		///< Регион данных.
		uorRect m_rectDataRegionFrame;	///< Рамка региона данных. А то что-то тоскливо выглядит...
		QSize  m_lastSize;

		uoReportDoc* 	m_rptDoc;
		QImage 			m_imageView;
		QImage 			m_imageViewCopy;
		int 			m_drawToImage;
		int 			m_paintLocker;
		bool			m_directDraw;	///< прамое рисование с пом. WinAPI или XLIb если такое возможно.

		int  m_maxVisibleLineNumberCnt; ///< Количество символов в максимальной видимой строке таблицы. Это нужно для вычисления ширины вертикальной линейки.

		bool m_showGroup;
		bool m_showSection;
		bool m_showRuler;

		bool m_saveWithSelection;

		bool m_showGrid;
		bool m_showFrame;
		bool m_showInvisiblChar;

		/// режимы взаимодействия с пользователем.
		uoReportUseMode 	m_useMode;
		uoReportStateMode 	m_stateMode;
		int m_resizeLine;	///< строка или колонка которая ресайзится.

		uoReportViewIteract* m_iteractView;
		uoReportSelection* 	 m_selections;

		QList<QShortcut*>	m_shortcutList;

		// группа контролирующая положение вьюва и его размеры/свойства.
		int m_rowsInPage; 		///< строк на страницу
		int m_colsInPage; 		///< столбцов на страницу

		int m_rowCountVirt;	///< виртуальные строки вьюва
		int m_colCountVirt;	///< виртуальные колонки вьюва

		int m_rowCountDoc;	///< строки дока (просто кеш)
		int m_colCountDoc;	///< колонки дока (просто кеш)


		uoRptGroupItemList* m_groupListCache;	///< кешь для экземпляров uoRptGroupItem
		uoRptGroupItemList* m_groupListV;		///< список ректов группировок столбцов
		uoRptGroupItemList* m_groupListH;		///< список ректов группировок строк

		uoRptSectionItemList* m_sectItemListCache;	///< кешь для экземпляров uoRptSectionItem
		uoRptSectionItemList* m_sectItemListV;		///< список итемов секций столбцов
		uoRptSectionItemList* m_sectItemListH;		///< список итемов секций строк

		rptScalePositionMap m_scaleStartPositionMapH; 	///< Координаты х() ячеек горизонтальной линейки (видимой части)
		rptScalePositionMap m_scaleStartPositionMapV;	///< Координаты y() ячеек вертикальной линейки (видимой части)

		uoReportPropEditor* m_propEditor;

		int m_sizeVvirt;	///< Виртуальный размер документа по вертикали. Виртуальный потому что может увеличиваться скролом.
		int m_sizeHvirt;	///< Виртуальный Размер документа по горизонтали
		int m_sizeVDoc;		///< Реальный размер документа.
		int m_sizeHDoc;		///< Реальный размер документа.
		int m_pageWidth;	///< Ширина страницы в столбцах стандартного размера
		int m_pageHeight;	///< Высота страницы в строках стандартного размера

	public slots:
		void changeDocSize(uorNumber sizeV, uorNumber sizeH);

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
