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
#include <QFrame>
#include <QRect>
#include <QRectF>
#include <QBrush>
#include <QPen>
#include <QContextMenuEvent>
#include <QScrollBar>
#include "uoReport.h"
#include "uoReportDoc.h"
#include "uoReportViewIteract.h"
#include "uoReportSelection.h"


namespace uoReport {


///\struct uoRptGroupItem
///\brief Координатное описание группировки строк/столбцов, подготовленное для рендринга.
struct uoRptGroupItem {
	uoRptGroupItem()
		: _folded(false)
		,_level(-1)
		,_start(-1)
		,_end(-1)
		,_id(-1)
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
	int _id;				///< Идентификатор гроуп итема.
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

		_id 	= spn->getId();
	}
	/// чистка итема....
	void clear(){
		_folded = false;
		_level = -1;
		_start = -1;
		_end = -1;
		_id = -1;
		_rectEndPos = 0.0;
		_rectMidlePos = 0.0;
		_sizeTail = 0.0;
		_rectIteract.setTop(0.0);
		_rectIteract.setRight(0.0);
		_rectIteract.setBottom(0.0);
		_rectIteract.setLeft(0.0);
	}
};

typedef QList<uoRptGroupItem*> rptGroupItemList;
typedef QMap<int, qreal> rptScalePositionMap; ///< словарь смещений ячеек линеек.

///\class uoReportCtrl виджет, обслуживающий отрисовку отчета в режиме разработки или использования печатной формы.
///\brief инструмент для рендринга отчета, его корректировки.
class uoReportCtrl : public QWidget
{
    Q_OBJECT
    public:
        uoReportCtrl(QWidget *parent = 0);
        virtual ~uoReportCtrl();

		void setDoc(uoReportDoc* rptDoc);
        void clear();

		///\todo Нужно на всякий пожарный так же добавить определение private конструктора копирования и оператора копирующего
		/// присваивания, чтобы невозможно было случайно контрол скопировать.
    protected:

        void paintEvent(QPaintEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void contextMenuEvent(QContextMenuEvent *event);
		void showEvent( QShowEvent* event );
		void resizeEvent ( QResizeEvent * event );
		void keyPressEvent ( QKeyEvent * event );
		void wheelEvent ( QWheelEvent * event );

		//-------------------------------------------------
		bool mousePressEventForGroup(QMouseEvent *event);
		bool mousePressEventForRuler(QMouseEvent *event);
		bool findScaleLocation(qreal posX, qreal posY, int &scaleNo, uoRptHeaderType rht);
		uoBorderLocType  scaleLocationInBorder(qreal pos, QRectF rect, uoRptHeaderType rht);
		void keyPressEventMoveCursor ( QKeyEvent * event );
		void updateThis();
		int _freezUpdate;	/// заморозить посылку сообщений на перерисовку

		// Акселераторы для поиска запчасти под курсором.
		uoRptSparesType _curMouseSparesType;
		int 			_curMouseSparesNo;
		uoRptHeaderType _curMouseSparesRht;
		QRectF			_curMouseSparesRect;
		/**
			последняя позиция мышки в которой было иницировано действие нажатием левой клавишы.
			Например иницировано начало выделения или изменения размера ячейки.
			если пользователь тут же отпускает мышку, но рука дергается возникает нежелательный
			еффект перетаскивания. Это надо исключить. и исключим с пом. этой _curMouseLastPos.
		*/
		QPoint			_curMouseLastPos;
		void 			mouseSparesAcceleratorDrop();
		void 			mouseSparesAcceleratorSave(uoRptSparesType spar, int nom, uoRptHeaderType rht);

		QPoint 	_curentCell; ///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		void	setCurentCell(int x, int y, bool ensureVisible = false);
		bool 	curentCellVisible();

	public slots:
		void	onSetVScrolPos(int y);
		void	onSetHScrolPos(int x);
		void	onScrollActionV(int act);
		void	onScrollActionH(int act);
		void	doScrollAction(int act, uoRptHeaderType rht);
		void 	scrollView(int dx, int dy);

    protected:

		//-------- draw section ----------------
		void drawWidget(QPainter& painter);
		void drawHeaderControlContour(QPainter& painter);
		void drawHeaderControlGroup(QPainter& painter);
		void drawHeaderControl(QPainter& painter);
		void drawDataArea(QPainter& painter);
		void drawCell(QPainter& painter, uoCell* cell, int row, int col, QRectF& rectCell);

		QBrush _brushWindow;
		QBrush _brushBase;
		QBrush _brushBlack;
		QBrush _brushSelection;
		QPen _penText;
		QPen _penNoPen;
		QPen _penWhiteText;
		QPen _penGrey;

		qreal _charWidthPlus; 	///< Опорная ширина символа "+" в текушем шрифте.
		qreal _charHeightPlus; 	///< Опорная высота символа "+" в текушем шрифте.

		qreal _scaleFactor;			///< Положительный соэффициент масштаба виджета, если он > 0, тогда виджет крупнее, если меньше, виджет мельче.
		qreal _scaleFactorO;		///< обратная величина фактора. для пересчетов смещений.

	private:
		void 		initControls(QWidget *parent);
		QScrollBar  *_vScrollCtrl;
		QScrollBar  *_hScrollCtrl;
		QWidget* 	_cornerWidget; //, _cornerWidget(parent)
		void 		recalcScrollBars();
	public:
		uoReportDoc* getDoc() {return _rptDoc;}
		bool saveDoc();
		bool saveDocAs();
		void optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler);

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
		uoRptGroupItem* getGropItemFromCache();

	private:
		uoReportDoc* _rptDoc;

	private:
		/// режимы взаимодействия с пользователем.
		uoReportUseMode 	_useMode;
		uoReportStateMode 	_stateMode;
		int _resizeLine;	///< строка или колонка которая ресайзится.
		void setStateMode(uoReportStateMode stMode);

	private:
		uoReportViewIteract* _iteractView;
		uoReportSelection* _selections;


	private slots:
		void debugRects();
		void onSave();
		void onSaveAs();
		void onGridShow();
		void onGridHide();

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

	private:
		/// данные/ректы для областей....
		QRectF _rectGroupV;		///< Вертикальные группировки
		QRectF _rectGroupH;		///< Горизонтальные группировки
		QRectF _rectSectionV;		///< Вертикальные секции
		QRectF _rectSectionH;		///< Горизонтальные секции
		QRectF _rectRulerV;		///< Вертикальная линейка
		QRectF _rectRulerH;		///< Горизонтальная линейка
		QRectF _rectRuleCorner;	///< Верхний корнер-виджет слева от горизонтальной и сверху от вертикальной линейки
		QRectF _rectAll;			///< Полный регион
		QRectF _rectDataRegion;		///< Регион данных.
		QRectF _rectDataRegionFrame;	///< Рамка региона данных. А то что-то тоскливо выглядит...


		int  _maxVisibleLineNumberCnt; ///< Количество символов в максимальной видимой строке таблицы. Это нужно для вычисления ширины вертикальной линейки.

		bool _showGroup;
		bool _showSection;
		bool _showRuler;
		bool _showGrid;
		bool _showFrame;

		//------- группа контролирующая положение вьюва и во вьюве.
		qreal _shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
		qreal _shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)

		int _firstVisible_RowTop; 	///< Первая верхняя видимая строка
		int _firstVisible_ColLeft; 	///< Первая левая видимая колонка

		int _lastVisibleRow; 	///< Последняя верхняя видимая строка
		int _lastVisibleCol; 	///< Последняя левая видимая колонка

		int _rowsInPage; 		///< строк на страницу
		int _colsInPage; 		///< столбцов на страницу

		int _rowCountVirt;	///< виртуальные строки вьюва
		int _colCountVirt;	///< виртуальные колонки вьюва

		int _rowCountDoc;	///< строки дока (просто кеш)
		int _colCountDoc;	///< колонки дока (просто кеш)

		void onAccessRowOrCol(int nom, uoRptHeaderType rht); ///< при доступе к строке или столбцу вьюва...
		void doChangeVirtualSize(uoRptHeaderType rht, int changeCnt); ///< обработать смену виртуального размера

		rptGroupItemList* _groupListCache;	///< кешь для экземпляров uoRptGroupItem
		rptGroupItemList* _groupListV;		///< список ректов группировок столбцов
		rptGroupItemList* _groupListH;		///< список ректов группировок строк

		rptScalePositionMap _scaleStartPositionMapH; 	///< Координаты х() ячеек горизонтальной линейки (видимой части)
		rptScalePositionMap _scaleStartPositionMapV;	///< Координаты y() ячеек вертикальной линейки (видимой части)

		int _sizeVvirt;	///< Виртуальный размер документа по вертикали. Виртуальный потому что может увеличиваться скролом.
		int _sizeHvirt;	///< Виртуальный Размер документа по горизонтали
		int _sizeVDoc;	///< Реальный размер документа.
		int _sizeHDoc;	///< Реальный размер документа.
		int _pageWidth;		///< Ширина страницы в столбцах стандартного размера
		int _pageHeight;	///< Высота страницы в строках стандартного размера




	public slots:
		void changeDocSize(qreal sizeV, qreal sizeH);

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
