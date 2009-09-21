/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTUNDO_H
#define UOREPORTUNDO_H

#include <QList>

#include "uoReportDoc.h"
//class uoReportDoc;

namespace uoReport {

/**
	\struct uoRUndoUnit - Абстрактный класс комманды отмены/повтора редактирования
	\brief Абстрактный класс комманды отмены/повтора редактирования
*/
struct uoRUndoUnit
{
	public:
		uoRUndoUnit():m_nomCh(-1){};
		virtual ~uoRUndoUnit();

		int m_nomCh; 					///< номер изменения, если у нас нет комплексного уинта, а изменения группируюися...
		virtual QString toString() = 0; ///< сериализация итема для просмотра и анализа.

		virtual bool undo(uoReportDoc* doc) = 0;
		virtual bool redo(uoReportDoc* doc) = 0;
		virtual void clear() = 0;
};
/**
	\struct uoRUndo01 - класс комманды отмены/повтора редактирования текста.
	\brief класс комманды отмены/повтора редактирования текста.
*/
struct uoRUndo01 : public uoRUndoUnit
{
	private:
		int m_row;
		int m_col;
		QString m_text;
	public:

	uoRUndo01(int row, int col, QString text)
		:uoRUndoUnit()
		,m_row(row), m_col(col),m_text(text){};

	virtual ~uoRUndo01(){};
	virtual QString toString();
	virtual bool undo(uoReportDoc* doc);
	virtual bool redo(uoReportDoc* doc);
	virtual void clear() {};
};

/**
	\struct uoRUndo01 - класс комманды отмены/повтора размера котонок или строк.
	\brief класс комманды отмены/повтора редактирования uoHeaderScale

	uoHeaderScale* m_headerV; ///< Вертикальный заголовок
	uoHeaderScale* m_headerH; ///< Горизонтальный заголовок
	struct uoRptNumLine.
		qreal _size;
		qreal _sizeDef;
		bool _hiden;
		bool _fixed;

*/
struct uoRUndo02 : public uoRUndoUnit
{
	private:
		int 				m_row_or_col;
		uoRptHeaderType  	m_hType;
		uorHeaderScaleChangeType m_changeType;
	public:
		union //m_data
		{
			bool 	mu_fixed;
			bool 	mu_hiden;
			qreal 	mu_size;
		};
	public:

	uoRUndo02(int row_or_col, uoRptHeaderType hType, uorHeaderScaleChangeType chType)
		:uoRUndoUnit()
		,m_row_or_col(row_or_col),m_hType(hType), m_changeType(chType){};

	virtual ~uoRUndo02(){};
	virtual QString toString();
	virtual bool undo(uoReportDoc* doc);
	virtual bool redo(uoReportDoc* doc);
	virtual void clear() {};
};


/**
	\class uoReportUndo - Основной класс хранения и обработки механизма ундо/редо.
	\brief Основной класс хранения и обработки механизма ундо/редо.
*/
class uoReportUndo
{
	public:
		uoReportUndo();
		virtual ~uoReportUndo();
		void setDoc(uoReportDoc* doc);
	protected:
		uoReportDoc* m_doc;

		bool m_collectChanges; 	/// Старт сборки изменений..
		int m_maxUndoCount;		/// Максимальное количество ундо комманд в стеке.
		int m_nextUndoNom;		/// Счетчик номеров добавляемой комманды.
		bool m_groupStarted;	/// сигнал на группировку следующих поступающих изменений

		void pushUndo(uoRUndoUnit* unit);

		QList<uoRUndoUnit*> m_undoStack;
		QList<uoRUndoUnit*> m_redoStack;

	public:

		/// Запрос на статус сборки изменений
		bool isCollectChanges() { 	return m_collectChanges; }

		void enableCollectChanges(const bool& enable);

		void groupCommandStart() 	{ ++m_nextUndoNom; m_groupStarted = true;}
		void groupCommandEnd()		{m_groupStarted = false;}

		bool undo(uoReportDoc* doc);
		bool redo(uoReportDoc* doc);

		bool undoAvailability();
		bool redoAvailability();

		void clear();
		void traceToDebug();

	public:
		void doTextChange(QString oldText, int row, int col);
		void doScaleResize(uoRptHeaderType hType, int nomRC, qreal oldSize);


};
} //namespace uoReport

#endif // UOREPORTUNDO_H
