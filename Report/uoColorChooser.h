/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOCOLORCHOOSER_H
#define UOCOLORCHOOSER_H

#include <QtGlobal>
#include <QWidget>
#include <QFrame>
#include <QLineEdit>
#include <QComboBox> // удалить попозжа...

class uoColorCContainer;

class uoColorChooser : public QFrame
{
	Q_OBJECT
	public:
		uoColorChooser(QWidget *parent = 0);
		virtual ~uoColorChooser();
	public:
		typedef enum uoCC_Btn{
			uoBtn_showPalette = 0
			, uoBtn_colorChoose
			, uoBtn_colorClear
			, uoBtn_User = 100
		};
		int addColor(QColor col);
		void fillColors();

//		int setColorDefault(QColor col);

		QColor getColor(int pos = -1);
		QRect getGlobalRect();
		QSize sizeHint() const;
		QSize minimumSizeHint() const;
		void recalcRects();
		void setCurentColor(int colNum);
		struct uoAddButton	{
			uoAddButton()
				:m_PE(QStyle::PE_CustomBase)
				{}
			public:
				QString m_text;
				QStyle::PrimitiveElement m_PE;
				int 	m_size;
				QRect	m_rect;
				uoCC_Btn m_Type;
				bool	m_pressed;
		};

		virtual bool eventFilter ( QObject * watched, QEvent * event );
	private:
		int addButton(const QString& text, uoCC_Btn type);
		int addButtonPrimitive(const QStyle::PrimitiveElement prim, uoCC_Btn type);
		void setupBotton();

    protected:
		void paintEvent(QPaintEvent *event);
		void resizeEvent ( QResizeEvent * event );
		void keyPressEvent ( QKeyEvent * event );
        void mouseReleaseEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void hideEvent(QHideEvent *);
        virtual void timerEvent(QTimerEvent* e);


		void focusInEvent ( QFocusEvent * event );
		void focusOutEvent ( QFocusEvent * event );
    protected:
		void popupShow();
		void popupHide();
		bool popupVisible();
		void resetButton();
		void checkButtonAction(uoAddButton* btn);

	private:
		QList<QColor> m_colorList;
		QList<uoAddButton*> m_btns;
		QList<QRect*> m_btnRect;
		int  m_allAddBtnSize;
		QRect m_colorArea;
		int m_itemInRow;

		QColor  m_colorCur;
		int  	m_colorCurPos;
		int m_addBtn; ///< Количество дополнительных кнопок
		int m_curCtrl; ///< Текущий контрол.
		uoColorCContainer* m_Container;
		uoColorCContainer* getContainer();

	friend class uoColorCContainer;


};

#endif // UOCOLORCHOOSER_H
