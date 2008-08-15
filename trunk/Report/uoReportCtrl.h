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


namespace uoReport {

///\enum uoReportUseMode - режим работы с отчетом разработка или использование
enum uoReportUseMode {
	rmDevelMode = 0
	, rmUsingMode
};

///\enum uoReportStateMode - режим взаимодействия с пользователем
/// Например: редактирование ячейки, выделение групп ячеек, и т.п.
enum uoReportStateMode {
	rmsNone = 0
	, rmsResizeRule_Top
	, rmsResizeRule_Left
};

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
        void clear();
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

    protected:
		//-------- draw section ----------------
		void drawWidget(QPainter& painter);
		void drawHeaderControl(QPainter& painter);
//		void drawHeaderControlGroup(QPainter& painter, uoRptHeaderType rhdrType);
		void drawDataArea(QPainter& painter);
		QBrush _brushWindow;
		QBrush _brushBase;
		QPen _penText;
		QPen _penNoPen;
		QPen _penWhiteText;

		rptSize _charWidthPlus; 	///< Опорная ширина символа "+" в текушем шрифте.
		rptSize _charHeightPlus; 	///< Опорная высота символа "+" в текушем шрифте.

		qreal _scaleFactor;			///< Положительный соэффициент масштаба виджета, если он > 0, тогда виджет крупнее, если меньше, виджет мельче.
		qreal _scaleFactorO;		///< обратная величина фактора. для пересчетов смещений.

		QPointF _pointScale;			///< Оттраслированный с пом. _scaleFactorO QPointF
		QRectF  _rectScale;				///< Оттраслированный с пом. _scaleFactorO QRectF
		void 	scalePoint(const QPoint& point); ///<Транслируется переданный point, результат в _pointScale
		void 	scaleRect(QRectF& rect);	///<Транслируется переданный rect, результат в _rectScale

	private:
		void initControls(QWidget *parent);
		QScrollBar  *_vScrollCtrl, *_hScrollCtrl;
		QWidget* _cornerWidget; //, _cornerWidget(parent)
	public:
		uoReportDoc* getDoc() {return _repoDoc;}
		bool saveDoc();
		bool saveDocAs();
		void optionShow(bool shGrid, bool shGroup, bool shSection, bool shRuler);

	protected:
		qreal getWidhtWidget() {return width() * (1/_scaleFactor);}; 	///< Ширина с учетом масштаба
		qreal getHeightWidget(){return height() * (1/_scaleFactor);};	///< Высота с учетом масштаба

		void recalcHeadersRects();
		void recalcGroupSectionRects(uoRptHeaderType rht = rhtUnknown);
		rptSize getLengthOfScale(uoRptHeaderType rht, int start, int stop);
		int  recalcVisibleScales(uoRptHeaderType rht);
		void calcGroupItemPosition(uoRptGroupItem* grItem, uoRptHeaderType rht);

		void dropGropItemToCache();
		uoRptGroupItem* getGropItemFromCache();

	private:
		uoReportDoc* _repoDoc;

	private:
		/// режимы взаимодействия с пользователем.
		uoReportUseMode 	_useMode;
		uoReportStateMode 	_stateMode;

	private:
		uoReportViewIteract* _iteractView;


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
		///\todo Добавить наруральну линейку для точного расчета местоположения встраиваемых объектов!!! кул идейка!!!!
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

		int _firstVisible_RowTop; 	///< Первая верхняя видимая строка
		int _firstVisible_ColLeft; 	///< Первая левая видимая колонка
		rptSize _shift_RowTop;		///< Смещение первой видимой строки вверх (грубо - размер невидимой/скрытой их части)
		rptSize _shift_ColLeft;		///< Смещение первой видимой колонки влево (грубо - размер невидимой/скрытой их части)

		int _lastVisibleRow; 	///< Первая верхняя видимая строка
		int _lastVisibleCol; 	///< Первая левая видимая колонка

		rptGroupItemList* _groupListCache;	///< кешь для экземпляров uoRptGroupItem
		rptGroupItemList* _groupListV;		///< список ректов группировок столбцов
		rptGroupItemList* _groupListH;		///< список ректов группировок строк

		rptScalePositionMap _scaleStartPositionMapH; 	///< Координаты х() ячеек горизонтальной линейки (видимой части)
		rptScalePositionMap _scaleStartPositionMapV;	///< Координаты y() ячеек вертикальной линейки (видимой части)

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
