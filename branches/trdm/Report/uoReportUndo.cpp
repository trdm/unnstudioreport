/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include <QDebug>
#include "uoReportUndo.h"

namespace uoReport {

uoRUndoUnit::~uoRUndoUnit()
{}

bool uoRUndo01::undo(uoReportDoc* doc){
	if(!doc)
		return false;
	QString txt = doc->getCellText(m_row, m_col);
	doc->setCellText(m_row, m_col, m_text);
	m_text = txt;
	return true;
}
bool uoRUndo01::redo(uoReportDoc* doc){
	if(!doc)
		return false;
	QString txt = doc->getCellText(m_row, m_col);
	doc->setCellText(m_row, m_col, m_text);
	m_text = txt;
	return true;
}
/**
	\class uoReportUndo - Основной класс хранения и обработки механизма ундо/редо.
	Сохраянет все данные для того, что-бы восстановить
	документ до того состояния,	которое было до комманды редактирования.

	Обшая стратегия такова: сохраняем значения изменяемого объекта,
	напримр текста в ячейке перед его изменением, имплементируем функию ундо
	для его восстановления. При ундо передаем документ и соответственно
	восстанавливаем данные, одновременно скидывая новые данные в комманду
	для имплементации РЕДО.

	Что и в каком виде надо сохранять, решу по ходу необходимости.
*/

uoReportUndo::uoReportUndo()
	: m_collectChanges(false)
	, m_maxUndoCount(UNDO_COMMAND_MAX_COUNT)
	, m_nextUndoNom(0)
	, m_groupStarted(false)
{
}
void uoReportUndo::setDoc(uoReportDoc* doc)
{
	m_doc = doc;
}
uoReportUndo::~uoReportUndo()
{
	clear();
}

bool uoReportUndo::undo(uoReportDoc* doc)
{
	if (m_undoStack.isEmpty())
		return false;
	bool rezult = false;
	bool old = m_collectChanges; m_collectChanges = false;

	uoRUndoUnit* uUnit = m_undoStack.takeLast();
	if (uUnit){
		int nCommand = uUnit->m_nomCh;

		do	{
			rezult = uUnit->undo(doc);
			if (!rezult) {
				delete uUnit;
				m_collectChanges = old;
				return rezult;
			}
			m_redoStack.append(uUnit);

			if (m_undoStack.isEmpty())
				break;
			uUnit = m_undoStack.takeLast();
			if (uUnit->m_nomCh != nCommand){
				m_undoStack.append(uUnit);
				break;
			}
		}
		while(true);
	}
	m_collectChanges = old;
	return rezult;
}

bool uoReportUndo::redo(uoReportDoc* doc)
{

	if (m_redoStack.isEmpty())
		return false;
	bool rezult = false;
	bool old = m_collectChanges; m_collectChanges = false;

	uoRUndoUnit* uUnit = m_redoStack.takeLast();
	if (uUnit){
		int nCommand = uUnit->m_nomCh;

		do	{
			rezult = uUnit->redo(doc);
			if (!rezult) {
				delete uUnit;
				m_collectChanges = old;
				return rezult;
			}
			m_undoStack.append(uUnit);

			if (m_redoStack.isEmpty())
				break;
			uUnit = m_redoStack.takeLast();
			if (uUnit->m_nomCh != nCommand){
				m_redoStack.append(uUnit);
				break;
			}
		}
		while(true);
	}
	m_collectChanges = old;
	return rezult;
}

void uoReportUndo::enableCollectChanges(const bool& enable)
{
	if (m_collectChanges && !enable)
		clear();
	m_collectChanges = enable;
}



bool uoReportUndo::undoAvailability(){	return !m_undoStack.isEmpty();}
bool uoReportUndo::redoAvailability(){	return !m_redoStack.isEmpty();}

void uoReportUndo::pushUndo(uoRUndoUnit* unit)
{
	if (m_undoStack.size()>m_maxUndoCount)	{
		delete m_undoStack.takeFirst();
	}
	if (!m_groupStarted)
		++m_nextUndoNom;
	unit->m_nomCh = m_nextUndoNom;

	m_undoStack.append(unit);
	if (!m_redoStack.isEmpty()){
		while(!m_redoStack.isEmpty())
			delete m_redoStack.takeFirst();
	}
}

void uoReportUndo::clear()
{
	while(!m_undoStack.isEmpty())
		delete m_undoStack.takeFirst();

	while(!m_redoStack.isEmpty())
		delete m_redoStack.takeFirst();

}


/// Вывод в оладку
void uoReportUndo::traceToDebug()
{
	qDebug() << " m_undoStack sz: " << m_undoStack.size();
}
void uoReportUndo::doTextChange(QString oldText, int row, int col)
{
	if (!m_collectChanges)
		return;
	uoRUndo01* undo = new uoRUndo01(row, col, oldText);
	if (undo){
		pushUndo(undo);
	}
}



} //namespace uoReport
