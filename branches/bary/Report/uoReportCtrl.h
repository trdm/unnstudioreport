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


//QAbstractScrollArea
///\struct uoRptGroupItem
///\brief Координатное описание группировки строк/столбцов
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
		{}
	QRectF 	_rectIteract; 	///< область "кнопки" свертки/развертки структуры.
	bool _folded; 			///< уровень группировки.
	int _level; 			///< уровень группировки.
	int _start; 			///< Начало диапазона.
	int _end;				///< Конец диапазона.
	int _id;				///< Идентификатор гроуп итема.
	qreal	_rectEndPos; 	///< Координаты правой|нижней стороны ректа. для расчета длины линии группировки.
	qreal	_rectMidlePos; 	///< Координаты середины правой|нижней стороны ректа для вычерчивания горизонтальной линии группировки.
	rptSize _sizeTail;		///< Размер "хвоста" группировки.

	/// копируем данные из uoLineSpan
	void copyFrom(uoLineSpan* spn){
		_start 	= spn->getStart();
		_end 	= spn->getEnd();

		_folded = spn->getFolded();
		_level	= spn->getLevel();

		_id 	= spn->getId();
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
		//-------------------------------------------------
		bool mousePressEventForGroup(QMouseEvent *event);
		bool mousePressEventForRuler(QMouseEvent *event);
		bool findScaleLocation(qreal posX, qreal posY, int &scaleNo, uoRptHeaderType rht);
		uoBorderLocType  scaleLocationInBorder(qreal pos, QRectF rect, uoRptHeaderType rht);

		// Акселераторы для поиска запчасти под курсором.
		uoRptSparesType _curMouseSparesType;
		int 			_curMouseSparesNo;
		uoRptHeaderType _curMouseSparesRht;
		QRectF			_curMouseSparesRect;
		void 			mouseSparesAcceleratorDrop();
		void 			mouseSparesAcceleratorSave(uoRptSparesType spar, int nom, uoRptHeaderType rht);

		QPoint 	_curentCell; ///< Текущая ячейка вьюва. есть всегда. Даже когда работаем с картинками.
		void	setCurentCell(int x, int y, bool ensureVisible = false);

    protected:

		//-------- draw section ----------------
		void drawWidget(QPainter& painter);
		void drawHeaderControlContour(QPainter& painter);
		void drawHeaderControlGroup(QPainter& painter);
		void drawHeaderControl(QPainter& painter);
		void drawDataArea(QPainter& painter);
		// BaryVetaL
		void drawBkImage(QPainter& painter);

		QBrush _brushWindow;
		QBrush _brushBase;
		QBrush _brushBlack;
		QPen _penText;
		QPen _penNoPen;
		QPen _penWhiteText;
		QPen _penGrey;

		// BaryVetaL
		QImage* _bkImage;
		QImage* _bkImageOriginal;

		rptSize _charWidthPlus; 	///< Опорная ширина символа "+" в текушем шрифте.
		rptSize _charHeightPlus; 	///< Опорная высота символа "+" в текушем шрифте.

		qreal _scaleFactor;			///< Положительный соэффициент масштаба виджета, если он > 0, тогда виджет крупнее, если меньше, виджет мельче.
		qreal _scaleFactorO;		///< обратная величина фактора. для пересчетов смещений.

	private:
		void initControls(QWidget *parent);
		QScrollBar  *_vScrollCtrl, *_hScrollCtrl;
		QWidget* _cornerWidget; //, _cornerWidget(parent)
	public:
		uoReportDoc* getDoc() {return _rptDoc;}
		bool saveDoc();
		bool saveDocAs();
		void optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler);

	protected:
		qreal getWidhtWidget() {return width() * (1/_scaleFactor);}; 	///< Ширина с учетом масштаба
		qreal getHeightWidget(){return height() * (1/_scaleFactor);};	///< Высота с учетом масштаба

		void recalcHeadersRects();
		void recalcGroupSectionRects(uoRptHeaderType rht = rhtUnknown);
		// BaryVetaL
		void recalcbkImage();
		rptSize getLengthOfScale(uoRptHeaderType rht, int start, int stop);
		int  recalcVisibleScales(uoRptHeaderType rht);
		void calcGroupItemPosition(uoRptGroupItem* grItem, uoRptHeaderType rht);

		void dropGropItemToCache();
		uoRptGroupItem* getGropItemFromCache();

	private:
		uoReportDoc* _rptDoc;

	private:
		/// режимы взаимодействия с пользователем.
		uoReportUseMode 	_useMode;
		uoReportStateMode 	_stateMode;

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
        // BaryVetaL
		void onLoadBkImage();

		void onOutToDebug();
		void onSetScaleFactor(const qreal sFactor);

	private:
		/// данные/ректы для областей....
		QRectF* _rectGroupV;		///< Вертикальные группировки
		QRectF* _rectGroupH;		///< Горизонтальные группировки
		QRectF* _rectSectionV;		///< Вертикальные секции
		QRectF* _rectSectionH;		///< Горизонтальные секции
		QRectF* _rectRulerV;		///< Вертикальная линейка
		QRectF* _rectRulerH;		///< Горизонтальная линейка
		QRectF* _rectRuleCorner;	///< Верхний корнер-виджет слева от горизонтальной и сверху от вертикальной линейки
		QRectF* _rectAll;			///< Полный регион
		QRectF* _rectDataRegion;	///< Регион данных.


		int  _maxVisibleLineNumberCnt; ///< Количество символов в максимальной видимой строке таблицы. Это нужно для вычисления ширины вертикальной линейки.

		bool _showGroup;
		bool _showSection;
		bool _showRuler;
		bool _showGrid;
		bool _showFrame;

		//------- группа контролирующая положение вьюва и во вьюве.
		rptSize _shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
		rptSize _shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)

		int _firstVisible_RowTop; 	///< Первая верхняя видимая строка
		int _firstVisible_ColLeft; 	///< Первая левая видимая колонка

		int _lastVisibleRow; 	///< Первая верхняя видимая строка
		int _lastVisibleCol; 	///< Первая левая видимая колонка

		qreal _sizeVvirt;	///< Виртуальный размер документа по вертикали
		qreal _sizeHvirt;	///< Виртуальный Размер документа по горизонтали


		rptGroupItemList* _groupListCache;	///< кешь для экземпляров uoRptGroupItem
		rptGroupItemList* _groupListV;		///< список ректов группировок столбцов
		rptGroupItemList* _groupListH;		///< список ректов группировок строк

		rptScalePositionMap _scaleStartPositionMapH; 	///< Координаты х() ячеек горизонтальной линейки (видимой части)
		rptScalePositionMap _scaleStartPositionMapV;	///< Координаты y() ячеек вертикальной линейки (видимой части)

	public slots:
		void setDocSize(int row, int col, qreal sizeV, qreal sizeH);

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
