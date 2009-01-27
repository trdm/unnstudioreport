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
		: _folded(false)
		,_level(-1)
		,_start(-1)
		,_end(-1)
		,m_id(-1)
		,_rectEndPos(0)
		,_rectMidlePos(0)
		,_sizeTail(0)
		,_tailPosIsAbs(false)
		{}
	QRectF 	_rectIteract; 	///< область "кнопки" свертки/развертки структуры.
	bool _folded; 			///< уровень группировки.
	int _level; 			///< уровень группировки.
	int _start; 			///< Начало диапазона.
	int _end;				///< Конец диапазона.
	int m_id;				///< Идентификатор гроуп итема.
	qreal	_rectEndPos; 	///< Координаты правой|нижней стороны ректа. для расчета длины линии группировки.
	qreal	_rectMidlePos; 	///< Координаты середины правой|нижней стороны ректа для вычерчивания горизонтальной линии группировки.
	qreal _sizeTail;		///< Размер "хвоста" группировки.
	bool 	_tailPosIsAbs;	///< Размер "хвоста" указан относительно левой или верхней сторони ректа группировок

	/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		_start 	= spn->getStart();
		_end 	= spn->getEnd();

		_folded = spn->getFolded();
		_level	= spn->getLevel();

		m_id 	= spn->getId();
	}
	/// чистка итема....
	void clear(){
		_folded = false;
		_level = -1;
		_start = -1;
		_end = -1;
		m_id = -1;
		_rectEndPos = 0.0;
		_rectMidlePos = 0.0;
		_sizeTail = 0.0;
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
		:_level(-1)
		,_start(-1)
		,_end(-1)
		,m_id(-1)
		,_nameSections(0)
		{}
	QRectF 	_rectView; 		///< область "кнопки" свертки/развертки структуры.
	int 	_level; 		///< уровень группировки.
	int 	_start; 		///< Начало диапазона.
	int 	_end;			///< Конец диапазона.
	int 	m_id;			///< Идентификатор гроуп итема.
	bool 	_selected;		///< Секция выделенна.
	QString _nameSections;	///< Имя секции.

		/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		_start 	= spn->getStart();
		_end 	= spn->getEnd();

		_level	= spn->getLevel();
		_nameSections	= spn->_name;

		m_id 	= spn->getId();
	}

	/// чистка итема....
	void clear(){
		_level = -1;
		_start = -1;
		_end = -1;
		m_id = -1;
		_nameSections = "";
		_selected = false;
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

		///\todo Нужно на всякий пожарный так же добавить определение private конструктора копирования и оператора копирующего
		/// присваивания, чтобы невозможно было случайно контрол скопировать.
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

		uoRptSparesType findPointLocation(qreal posX, qreal posY);

		uoBorderLocType  scaleLocationInBorder(qreal pos, QRectF rect, uoRptHeaderType rht);
		void keyPressEventMoveCursor ( QKeyEvent * event );
		void updateThis();
		int m_freezUpdate;	/// заморозить посылку сообщений на перерисовку

		// Акселераторы для поиска запчасти под курсором.
		uoRptSparesType m_curMouseSparesType;
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
		void 			mouseSparesAcceleratorSave(uoRptSparesType spar, int nom, uoRptHeaderType rht);

		QPoint 	m_curentCell; ///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		QRect 	m_curentCellRect; ///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		void	setCurentCell(int x, int y, bool ensureVisible = false);
		QRect 	getCellRect(const int& posY, const int& posX);
		QPoint 	getCellFromPosition(const qreal& posY, const qreal& posX);
		bool 	curentCellVisible();

		bool propertyEditorApply();
		bool propertyEditorVisible();

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
		void onSelectonChange(const uoRptSelectionType& sModeOld, const uoRptSelectionType& sModeNew);


	public:
		uoReportDoc* getDoc() {return m_rptDoc;}
		bool saveDoc();
		bool saveDocAs();
		void optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler);
		uoReportSelection* getSelection(){ return m_selections;};
		const QPoint 	getCurentCell() const {return m_curentCell;}
		bool propertyEditorInit(uoReportPropEditor* pPropEditor);
		bool propertyEditorApply(uoReportPropEditor* pPropEditor);
		bool populatePropEditor(uoReportPropEditor* propEditor);


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
		uoReportDoc* 	m_rptDoc;
		QImage 			m_imageView;
		int 			m_drawToImage;
		void updateImage();

		// В эти переменные собираем свойства выделнных элементов.
		uorTextDecor 		m_textDec_Selection;
		uorBorderPropBase 	m_borderProp_Selection;	///< закешированное значения свойств.
		void recalcSelectionProperty();

	private:
		/// режимы взаимодействия с пользователем.
		uoReportUseMode 	m_useMode;
		uoReportStateMode 	m_stateMode;
		int _resizeLine;	///< строка или колонка которая ресайзится.
		void setStateMode(uoReportStateMode stMode);

	private:
		uoReportViewIteract* m_iteractView;
		uoReportSelection* 	 m_selections;
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
		void propertyEditorHide();
		void onPropChange();

	private:
		void onSpanInclude(uoRptSpanType spType);
		void onSpanExclude(uoRptSpanType spType);

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


		int  m_maxVisibleLineNumberCnt; ///< Количество символов в максимальной видимой строке таблицы. Это нужно для вычисления ширины вертикальной линейки.

		bool m_showGroup;
		bool m_showInvisiblChar;
		bool m_showSection;
		bool m_showRuler;
		bool m_showGrid;
		bool m_showFrame;

		//------- группа контролирующая положение вьюва и во вьюве.
		qreal m_shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
		qreal _shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)

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

		void onAccessRowOrCol(int nom, uoRptHeaderType rht); ///< при доступе к строке или столбцу вьюва...
		void onAccessRowCol(int nmRow = 0, int nmCol = 0); ///< при доступе к строке или столбцу вьюва...
		void doChangeVirtualSize(uoRptHeaderType rht, int changeCnt); ///< обработать смену виртуального размера

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
		int m_sizeVDoc;	///< Реальный размер документа.
		int m_sizeHDoc;	///< Реальный размер документа.
		int m_pageWidth;		///< Ширина страницы в столбцах стандартного размера
		int m_pageHeight;	///< Высота страницы в строках стандартного размера

	public slots:
		void changeDocSize(qreal sizeV, qreal sizeH);

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
