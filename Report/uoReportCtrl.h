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
///\brief ������������ �������� ����������� �����/��������
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
	QRectF 	_rectIteract; 	///< ������� "������" �������/��������� ���������.
	bool _folded; 			///< ������� �����������.
	int _level; 			///< ������� �����������.
	int _start; 			///< ������ ���������.
	int _end;				///< ����� ���������.
	int _id;				///< ������������� ����� �����.
	qreal	_rectEndPos; 	///< ���������� ������|������ ������� �����. ��� ������� ����� ����� �����������.
	qreal	_rectMidlePos; 	///< ���������� �������� ������|������ ������� ����� ��� ������������ �������������� ����� �����������.
	rptSize _sizeTail;		///< ������ "������" �����������.

	/// �������� ������ �� uoLineSpan
	void copyFrom(uoLineSpan* spn){
		_start 	= spn->getStart();
		_end 	= spn->getEnd();

		_folded = spn->getFolded();
		_level	= spn->getLevel();

		_id 	= spn->getId();
	}
};

typedef QList<uoRptGroupItem*> rptGroupItemList;
typedef QMap<int, qreal> rptScalePositionMap; ///< ������� �������� ����� ������.

///\class uoReportCtrl ������, ������������� ��������� ������ � ������ ���������� ��� ������������� �������� �����.
///\brief ���������� ��� ��������� ������, ��� �������������.
class uoReportCtrl : public QWidget
{
    Q_OBJECT
    public:
        uoReportCtrl(QWidget *parent = 0);
        virtual ~uoReportCtrl();

		void setDoc(uoReportDoc* rptDoc);
        void clear();

		///\todo ����� �� ������ �������� ��� �� �������� ����������� private ������������ ����������� � ��������� �����������
		/// ������������, ����� ���������� ���� �������� ������� �����������.
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

		// ������������ ��� ������ �������� ��� ��������.
		uoRptSparesType _curMouseSparesType;
		int 			_curMouseSparesNo;
		uoRptHeaderType _curMouseSparesRht;
		QRectF			_curMouseSparesRect;
		void 			mouseSparesAcceleratorDrop();
		void 			mouseSparesAcceleratorSave(uoRptSparesType spar, int nom, uoRptHeaderType rht);

		QPoint 	_curentCell; ///< ������� ������ �����. ���� ������. ���� ����� �������� � ����������.
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

		rptSize _charWidthPlus; 	///< ������� ������ ������� "+" � ������� ������.
		rptSize _charHeightPlus; 	///< ������� ������ ������� "+" � ������� ������.

		qreal _scaleFactor;			///< ������������� ����������� �������� �������, ���� �� > 0, ����� ������ �������, ���� ������, ������ ������.
		qreal _scaleFactorO;		///< �������� �������� �������. ��� ���������� ��������.

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
		qreal getWidhtWidget() {return width() * (1/_scaleFactor);}; 	///< ������ � ������ ��������
		qreal getHeightWidget(){return height() * (1/_scaleFactor);};	///< ������ � ������ ��������

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
		/// ������ �������������� � �������������.
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
		/// ������/����� ��� ��������....
		QRectF* _rectGroupV;		///< ������������ �����������
		QRectF* _rectGroupH;		///< �������������� �����������
		QRectF* _rectSectionV;		///< ������������ ������
		QRectF* _rectSectionH;		///< �������������� ������
		QRectF* _rectRulerV;		///< ������������ �������
		QRectF* _rectRulerH;		///< �������������� �������
		QRectF* _rectRuleCorner;	///< ������� ������-������ ����� �� �������������� � ������ �� ������������ �������
		QRectF* _rectAll;			///< ������ ������
		QRectF* _rectDataRegion;	///< ������ ������.


		int  _maxVisibleLineNumberCnt; ///< ���������� �������� � ������������ ������� ������ �������. ��� ����� ��� ���������� ������ ������������ �������.

		bool _showGroup;
		bool _showSection;
		bool _showRuler;
		bool _showGrid;
		bool _showFrame;

		//------- ������ �������������� ��������� ����� � �� �����.
		rptSize _shift_RowTop;		///< �������� ������ ������� ������ ����� (����� - ������ ���������/������� �� �����)
		rptSize _shift_ColLeft;		///< �������� ������ ������� ������� ����� (����� - ������ ���������/������� �� �����)

		int _firstVisible_RowTop; 	///< ������ ������� ������� ������
		int _firstVisible_ColLeft; 	///< ������ ����� ������� �������

		int _lastVisibleRow; 	///< ������ ������� ������� ������
		int _lastVisibleCol; 	///< ������ ����� ������� �������

		qreal _sizeVvirt;	///< ����������� ������ ��������� �� ���������
		qreal _sizeHvirt;	///< ����������� ������ ��������� �� �����������


		rptGroupItemList* _groupListCache;	///< ���� ��� ����������� uoRptGroupItem
		rptGroupItemList* _groupListV;		///< ������ ������ ����������� ��������
		rptGroupItemList* _groupListH;		///< ������ ������ ����������� �����

		rptScalePositionMap _scaleStartPositionMapH; 	///< ���������� �() ����� �������������� ������� (������� �����)
		rptScalePositionMap _scaleStartPositionMapV;	///< ���������� y() ����� ������������ ������� (������� �����)

	public slots:
		void setDocSize(int row, int col, qreal sizeV, qreal sizeH);

};

} //namespace uoMetaData

#endif // UOREPTOPRULER_H
