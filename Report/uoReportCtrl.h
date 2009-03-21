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
#include "uoReport.h"
#include "uoReportDocBody.h"
#include "uoReportViewIteract.h"
#include "uoReportSelection.h"
#include "uoReportPropEditor.h"

class uoReportDoc;
class uoReportUndo;
struct uoCell;
struct uoEnumeratedItem;
struct uorTextDecor;
struct uorBorderPropBase;

namespace uoReport {


///\struct uoRptGroupItem
///\brief Координатное описание группировки строк/столбцов, подготовленное для рендринга.
struct uoRptGroupItem {
	uoRptGroupItem()
		: m_folded(false)
		,m_level(-1)
		,m_start(-1)
		,m_end(-1)
		,m_id(-1)
		,m_rectEndPos(0)
		,m_rectMidlePos(0)
		,m_sizeTail(0)
		,m_tailPosIsAbs(false)
		{}
	QRectF 	_rectIteract; 	///< область "кнопки" свертки/развертки структуры.
	bool m_folded; 			///< уровень группировки.
	int m_level; 			///< уровень группировки.
	int m_start; 			///< Начало диапазона.
	int m_end;				///< Конец диапазона.
	int m_id;				///< Идентификатор гроуп итема.
	qreal	m_rectEndPos; 	///< Координаты правой|нижней стороны ректа. для расчета длины линии группировки.
	qreal	m_rectMidlePos; 	///< Координаты середины правой|нижней стороны ректа для вычерчивания горизонтальной линии группировки.
	qreal m_sizeTail;		///< Размер "хвоста" группировки.
	bool 	m_tailPosIsAbs;	///< Размер "хвоста" указан относительно левой или верхней сторони ректа группировок

	/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		m_start 	= spn->getStart();
		m_end 	= spn->getEnd();

		m_folded = spn->getFolded();
		m_level	= spn->getLevel();

		m_id 	= spn->getId();
	}
	/// чистка итема....
	void clear(){
		m_folded = false;
		m_level = -1;
		m_start = -1;
		m_end = -1;
		m_id = -1;
		m_rectEndPos = 0.0;
		m_rectMidlePos = 0.0;
		m_sizeTail = 0.0;
		_rectIteract.setTop(0.0);
		_rectIteract.setRight(0.0);
		_rectIteract.setBottom(0.0);
		_rectIteract.setLeft(0.0);
	}
};

typedef QList<uoRptGroupItem*> uoRptGroupItemList;
typedef QMap<int, qreal> rptScalePositionMap; ///< словарь смещений ячеек линеек.

/**
	\struct uoRptSectionItem - структура для сохранения гуи-координат и атрибутов секции отчета.
	\brief Используется в uoReportCtr для отрисовки секций.
*/
struct uoRptSectionItem
{
		uoRptSectionItem()
		:m_level(-1)
		,m_start(-1)
		,m_end(-1)
		,m_id(-1)
		,m_nameSections(0)
		{}
	QRectF 	_rectView; 		///< область "кнопки" свертки/развертки структуры.
	int 	m_level; 		///< уровень группировки.
	int 	m_start; 		///< Начало диапазона.
	int 	m_end;			///< Конец диапазона.
	int 	m_id;			///< Идентификатор гроуп итема.
	bool 	m_selected;		///< Секция выделенна.
	QString m_nameSections;	///< Имя секции.

		/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		m_start 	= spn->getStart();
		m_end 	= spn->getEnd();

		m_level	= spn->getLevel();
		m_nameSections	= spn->_name;

		m_id 	= spn->getId();
	}

	/// чистка итема....
	void clear(){
		m_level = -1;
		m_start = -1;
		m_end = -1;
		m_id = -1;
		m_nameSections = "";
		m_selected = false;
		_rectView.setTop(0.0);
		_rectView.setRight(0.0);
		_rectView.setBottom(0.0);
		_rectView.setLeft(0.0);
	}
};

typedef QList<uoRptSectionItem*> uoRptSectionItemList;


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
		:m_ctrl(0),m_needCreate(false)
	{
		m_textDec_Selection.resetItem();
		m_borderProp_Selection.resetItem();
	}
	uoReportCtrl* m_ctrl;
	virtual bool visit(uoCell* cell, bool& first = false) = 0;
	bool m_needCreate;
	uorTextDecor 		m_textDec_Selection;
	uorBorderPropBase 	m_borderProp_Selection;	///< закешированное значения свойств.


};

/**
	\struct uorSelVisitorSaveProps - обходчик+сборшик пропертей по выделениям
	\brief обходчик+сборшик пропертей по выделениям
*/
struct uorSelVisitorSaveProps : public uorSelVisitorBase
{
	uorSelVisitorSaveProps(){}
	virtual bool visit(uoCell* cell, bool& first = false);
};

/**
	\struct uorSelVisitorSaveProps - обходчик+сборшик пропертей по выделениям
	\brief обходчик+сборшик пропертей по выделениям
*/
struct uorSelVisitorSetProps : public uorSelVisitorBase
{
	uorSelVisitorSetProps(){}
	virtual bool visit(uoCell* cell, bool& first = false);
};


/**
	\class uoReportCtrl виджет, обслуживающий отрисовку отчета в режиме разработки или использования печатной формы.
	\brief инструмент для рендринга отчета, его корректировки.
*/
class uoReportCtrl : public QWidget
{
    Q_OBJECT
    friend class uoReportDoc;
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
		bool findScaleLocation(qreal posX, qreal posY, int &scaleNo, uoRptHeaderType rht);

		uorSparesType findPointLocation(qreal posX, qreal posY);

		uorBorderLocType  scaleLocationInBorder(qreal pos, QRectF rect, uoRptHeaderType rht);
		void keyPressEventMoveCursor ( QKeyEvent * event );
		void updateThis();
		int m_freezUpdate;	/// заморозить посылку сообщений на перерисовку

		// Акселераторы для поиска запчасти под курсором.
		uorSparesType 	m_curMouseSparesType;
		int 			m_curMouseSparesNo;
		uoRptHeaderType m_curMouseSparesRht;
		QRectF			m_curMouseSparesRect;
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
		void	setCurentCell(int x, int y, bool ensureVisible = false);
		QRect 	getCellRect(const int& posY, const int& posX);
		QPoint 	getCellFromPosition(const qreal& posY, const qreal& posX);
		bool 	curentCellVisible();

	public slots:
		void	onSetVScrolPos(int y);
		void	onSetHScrolPos(int x);
		void	onScrollActionV(int act);
		void	onScrollActionH(int act);
		void	doScrollAction(int act, uoRptHeaderType rht);
		void 	scrollView(int dx, int dy);

	signals:
		void onColumnSizeChange(const int& nmColl, const qreal& nmLen);
		void onSelectionChange();
		void onCurentCellChange(const QPoint& oldPoint, const QPoint& newPoint);
		void onStateModeChange(const uoReportStateMode& oMode, const uoReportStateMode& nMode);


    protected:

		//-------- draw section ----------------
		void drawWidget(QPainter& painter);
		void drawHeaderControlContour(QPainter& painter);
		void drawHeaderControlGroup(QPainter& painter);
		void drawHeaderControl(QPainter& painter);
		void drawDataArea(QPainter& painter);
		void drawDataAreaResizeRuler(QPainter& painter);
		void drawCell(QPainter& painter, uoCell* cell, QRectF& rectCell, uoReportDoc* doc, bool isSell);

		QBrush m_brushWindow;
		QBrush m_brushBase;
		QBrush m_brushBlack;
		QBrush m_brushSelection;
		QPen m_penText;
		QPen m_penNoPen;
		QPen m_penWhiteText;
		QPen m_penGrey;

		qreal m_charWidthPlus; 	///< Опорная ширина символа "+" в текушем шрифте.
		qreal m_charHeightPlus; 	///< Опорная высота символа "+" в текушем шрифте.

		qreal m_scaleFactor;			///< Положительный соэффициент масштаба виджета, если он > 0, тогда виджет крупнее, если меньше, виджет мельче.
		qreal m_scaleFactorO;		///< обратная величина фактора. для пересчетов смещений.

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


	protected:
		qreal 	getWidhtWidget(); 	///< Ширина с учетом масштаба
		qreal 	getHeightWidget();	///< Высота с учетом масштаба

		bool 	rowVisible(int nmRow) const;
		bool 	colVisible(int nmCol) const;

		void 	recalcHeadersRects();
		void 	recalcGroupSectionRects(uoRptHeaderType rht = rhtUnknown);
		qreal getLengthOfScale(uoRptHeaderType rht, int start, int stop);
		int  	recalcVisibleScales(uoRptHeaderType rht);
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
		void onSetScaleFactor(const qreal sFactor);

		void onSectionInclude();
		void onSectionExclude();

		void onGroupInclude();
		void onGroupExclude();

		void onUndo();
		void onRedo();

		void propertyEditorShow();
		void propertyEditorShowActivate();
		void propertyEditorHide();
		void onPropChange();

	private:
		void onSpanInclude(uoRptSpanType spType);
		void onSpanExclude(uoRptSpanType spType);

		void onAccessRowOrCol(int nom, uoRptHeaderType rht); ///< при доступе к строке или столбцу вьюва...
		void onAccessRowCol(int nmRow = 0, int nmCol = 0); ///< при доступе к строке или столбцу вьюва...
		void doChangeVirtualSize(uoRptHeaderType rht, int changeCnt); ///< обработать смену виртуального размера

	private:
		/// данные/ректы для областей....
		QRectF m_rectGroupV;		///< Вертикальные группировки
		QRectF m_rectGroupH;		///< Горизонтальные группировки
		QRectF m_rectSectionV;		///< Вертикальные секции
		QRectF m_rectSectionH;		///< Горизонтальные секции
		QRectF m_rectRulerV;		///< Вертикальная линейка
		QRectF m_rectRulerH;		///< Горизонтальная линейка
		QRectF m_rectRuleCorner;	///< Верхний корнер-виджет слева от горизонтальной и сверху от вертикальной линейки
		QRectF m_rectAll;			///< Полный регион
		QRectF m_rectDataRegion;		///< Регион данных.
		QRectF m_rectDataRegionFrame;	///< Рамка региона данных. А то что-то тоскливо выглядит...

		uoReportDoc* 	m_rptDoc;
		QImage 			m_imageView;
		int 			m_drawToImage;

		int  m_maxVisibleLineNumberCnt; ///< Количество символов в максимальной видимой строке таблицы. Это нужно для вычисления ширины вертикальной линейки.

		bool m_showGroup;
		bool m_showInvisiblChar;
		bool m_showSection;
		bool m_showRuler;
		bool m_showGrid;
		bool m_showFrame;

		/// режимы взаимодействия с пользователем.
		uoReportUseMode 	m_useMode;
		uoReportStateMode 	m_stateMode;
		int m_resizeLine;	///< строка или колонка которая ресайзится.

		uoReportViewIteract* m_iteractView;
		uoReportSelection* 	 m_selections;


		// группа контролирующая положение вьюва и его размеры/свойства.
		qreal m_shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
		qreal m_shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)

		int m_firstVisible_RowTop; 	///< Первая верхняя видимая строка
		int m_firstVisible_ColLeft; 	///< Первая левая видимая колонка

		int m_lastVisibleRow; 	///< Последняя верхняя видимая строка
		int m_lastVisibleCol; 	///< Последняя левая видимая колонка

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
		void changeDocSize(qreal sizeV, qreal sizeH);

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
