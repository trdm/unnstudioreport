/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTLOADER_H
#define UOREPORTLOADER_H

#include "uoReport.h"

#include <QString>
#include <QMap>
#include <QTextStream>
#include <QTextDocument>
#include <QDataStream>
#include <QFile>
#include <QtXml>
#include <QProgressDialog>

class uoReportSelection;
class uoCellMatrix;

namespace uoReport {
// Loader - 1) грузчик
class uoReportLoader
{
	public:
		virtual ~uoReportLoader();

		static uoReportLoader* getLoader(uoRptStoreFormat stFormat);
		static QMap<uoRptStoreFormat,QString> getAviableLoadFormat(QString& filter);
		static QMap<uoRptStoreFormat,QString> getAviableStoreFormat(QString& filter);
		static uoRptStoreFormat getFormatByName(QString fileName);

		void 			setFileName(QString fileName);
		void 			setFormat(uoRptStoreFormat stFormat) {m_storeFormat = stFormat;}
		QString				getFileName();// {return m_docFilePath;};
		uoRptStoreFormat	getFormat() const {return m_storeFormat;}

		QString getLastError() const {return _lastError;}
		void 	setLastError(QString errMes) {_lastError = errMes;}

		virtual QString readAll() { return QString("");}
        virtual  bool readAll(QByteArray& baArr) {Q_UNUSED(baArr); return false;}
		QByteArray& getByteArray() {return m_byteArray;}


		bool validateStoreProps(bool forLoad = true);

		virtual bool initFO(bool forLoad = true) = 0; ///<Инициализация лоадера.
        /// Инициализация лоадера.
        virtual bool initBA(uorStoreOperation sOper, QByteArray& byteArray){
            Q_UNUSED(sOper); Q_UNUSED(byteArray); return false;
        }
//		virtual bool initBA(uorStoreOperation sOper, QByteArray	byteArray){return false;}; ///<Инициализация лоадера.
		virtual bool finalize() = 0; 				///<Деинициализация лоадера.

		/*
			Прегружаемые функции, реализация которых обязательна
			для того что-бы лоадер смог выгрузить или загрузить
			информацию в файл...
		*/

		/// Запись титульной части
		virtual bool saveDocStart(uoReportDoc* doc) = 0;

		/// Запись данных хейдера: секции, группировки
		virtual bool saveGroupsHeaderStart(int count, uoRptHeaderType rht) = 0;
		virtual bool saveGroupsItem(uoLineSpan* peSpn) = 0; /// Запись данных хейдера: секции, группировки
		virtual bool saveGroupsHeaderEnd(uoRptHeaderType rht) = 0;

		virtual bool saveSectionHeaderStart(int count, uoRptHeaderType rht) = 0;
		virtual bool saveSectionItem(uoLineSpan* peSpn) = 0;
		virtual bool saveSectionHeaderEnd(uoRptHeaderType rht) = 0;

		virtual bool saveScaleHeaderStart(int count, uoRptHeaderType rht) = 0;
		virtual bool saveScaleItem(uoRptNumLine* rLine) = 0;
		virtual bool saveScaleHeaderEnd(uoRptHeaderType rht) = 0;

		virtual void saveRowsStart(int rowCount) = 0;
//		virtual void saveRowItemStart(int rowNumb, int cellCount) = 0;
		virtual void saveRowItemStart(uoRow* row) = 0;

		virtual void saveCell(uoCell* cellItem) = 0;

		virtual void saveRowItemEnd() = 0;
		virtual void saveRowsEnd() = 0;

        virtual void saveSelectionStart(uorSelectionType selectionType){ Q_UNUSED(selectionType); }
        virtual void saveSelectionRCStart(uorSelectionType selectionType, int count){ Q_UNUSED(count); Q_UNUSED(selectionType); }
        virtual void saveSelectionRCItems(QList<int>& int_list){ Q_UNUSED(int_list); }
		virtual void saveSelectionRCEnd(){}
        virtual void saveSelectionCellsStart(uorSelectionType selectionType, int count){ Q_UNUSED(selectionType);  Q_UNUSED(count); }
        virtual void saveSelectionCell(const QPoint& point, int index, int count){  Q_UNUSED(point);  Q_UNUSED(index);  Q_UNUSED(count); }
		virtual void saveSelectionCellsEnd(){}
		virtual void saveSelectionEnd(){}

        virtual void saveMatrixStart(uoCellMatrix* matrx){ Q_UNUSED(matrx); }
        virtual void saveMatrixRowStart(int row){ Q_UNUSED(row); }
		virtual void saveMatrixRowEmd(){}
        virtual void saveMatrixEnd(uoCellMatrix* matrx){ Q_UNUSED(matrx); }


		virtual void saveFontStart(int count) = 0;
		virtual void saveFont(QFont* psFont, int nom) = 0;
		virtual void saveFontEnd() = 0;

		/// Запись произвольного блока строк...
		virtual void saveArbitraryBlockStart(QString blockName){} // arbitrary - произвольный
		virtual void saveArbitraryBlockItem(QString itemString){} // arbitrary - произвольный
		virtual void saveArbitraryBlockEnd(QString blockName){} // arbitrary - произвольный

		/// Запись подвальной части
		virtual bool saveDocEnd(uoReportDoc* doc) = 0;

		///================================================
		///===============LOAD=============================
		virtual bool load(uoReportDoc* doc) = 0;
		// остальное помоему можно пока и в производном классе сделать... пока...

		///===============LOAD=============================
		///================================================
        virtual void enableProcDialog(bool enable = true){Q_UNUSED(enable); }

	private:
		QString 		 _lastError;	///< Имя последней ошибки..
		QString 		 m_docFilePath;	///< Имя файла
	protected:
		uoReportDoc* 		m_doc;
		uoReportSelection* 	m_selection;	///< Класс вылеления, если не пуст, то фильтровать вывод
		QByteArray			m_byteArray;
		QString 		 	m_version;		///< Версия
		uoRptStoreFormat 	m_storeFormat;	///< Формат хранения файла отчета.
		uorStoreOperation 	m_storeOper;

};

class uoReportLoaderXML : public uoReportLoader {
	public:
		uoReportLoaderXML();
		virtual ~uoReportLoaderXML(){m_doc = NULL;}

		virtual bool initFO(bool forLoad = true);
		virtual bool initBA(uorStoreOperation sOper, QByteArray& byteArray); ///<Инициализация лоадера.

		virtual bool finalize();
		/// Запись титульной части
		virtual bool saveDocStart(uoReportDoc* doc);
		/// Запись данных хейдера: секции, группировки
		virtual bool saveGroupsHeaderStart(int count, uoRptHeaderType rht);
		virtual bool saveGroupsItem(uoLineSpan* peSpn);
		virtual bool saveGroupsHeaderEnd(uoRptHeaderType rht);

		virtual bool saveSectionHeaderStart(int count, uoRptHeaderType rht);
		virtual bool saveSectionItem(uoLineSpan* peSpn);
		virtual bool saveSectionHeaderEnd(uoRptHeaderType rht);

		virtual bool saveScaleHeaderStart(int count, uoRptHeaderType rht);
		virtual bool saveScaleItem(uoRptNumLine* rLine);
		virtual bool saveScaleHeaderEnd(uoRptHeaderType rht);

		virtual void saveRowsStart(int rowCount);
//		virtual void saveRowItemStart(int rowNumb, int cellCount);
		virtual void saveRowItemStart(uoRow* row);
		virtual void saveCell(uoCell* cellItem);
		virtual void saveRowItemEnd();
		virtual void saveRowsEnd();

		virtual void saveSelectionStart(uorSelectionType selectionType);
		virtual void saveSelectionRCStart(uorSelectionType selectionType, int count);
		virtual void saveSelectionRCItems(QList<int>& int_list);
		virtual void saveSelectionRCEnd();
		virtual void saveSelectionCellsStart(uorSelectionType selectionType, int count);
		virtual void saveSelectionCell(const QPoint& point, int index, int count);
		virtual void saveSelectionCellsEnd();
		virtual void saveSelectionEnd();

		virtual void saveMatrixStart(uoCellMatrix* matrx);
		virtual void saveMatrixRowStart(int row);
		virtual void saveMatrixRowEmd();
		virtual void saveMatrixEnd(uoCellMatrix* matrx);

		virtual void saveFontStart(int count);
		virtual void saveFont(QFont* psFont, int nom);
		virtual void saveFontEnd();

		// вот дуамю, стоит ли сделать защиту. наверное ненадо...
		virtual void saveArbitraryBlockStart(QString blockName); // arbitrary - произвольный
		virtual void saveArbitraryBlockItem(QString itemString);
		virtual void saveArbitraryBlockEnd(QString blockName);


		/// Запись подвальной части
		virtual bool saveDocEnd(uoReportDoc* doc);

		virtual bool load(uoReportDoc* doc);
		virtual  QString readAll();// { return QString("");
		virtual  bool readAll(QByteArray& baArr);// { return QString("");

		bool loadGroupsHeader(const QDomElement &node, uoReportDoc* doc);
		bool loadSectionHeader(const QDomElement &node, uoReportDoc* doc);
		bool loadScalesHeader(const QDomElement &node, uoReportDoc* doc);
		bool loadRows(const QDomElement &node, uoReportDoc* doc);
		bool loadCellsByRow(const QDomElement &node, uoReportDoc* doc, int rowNum);
		uoCell* loadCell(const QDomElement &node, uoReportDoc* doc, int rowNum);
		bool loadFont(const QDomElement &node, uoReportDoc* doc);

		bool loadMatrix(const QDomElement &node, uoReportDoc* doc);
		bool loadSelections(const QDomElement &node, uoReportDoc* doc);

		virtual void enableProcDialog(bool enable = true) {m_enableProcDialog = enable;}

	private:
		void save(QString str);

		bool 		endProcessRow(int row);
		QTextStream _textStream;
		QFile 		_outFile;
		QProgressDialog* m_progresDlg;
		bool 	m_enableProcDialog;
};

class uoReportLoaderTXT : public uoReportLoader {
	public:
		uoReportLoaderTXT();
		virtual ~uoReportLoaderTXT(){m_doc = NULL;}

		virtual bool initFO(bool forLoad = true);
		virtual bool initBA(uorStoreOperation sOper, QByteArray& byteArray); ///<Инициализация лоадера.

		virtual bool finalize();
		/// Запись титульной части
		virtual bool saveDocStart(uoReportDoc* doc);
		/// Запись данных хейдера: секции, группировки
		virtual bool saveGroupsHeaderStart(int count, uoRptHeaderType rht);
		virtual bool saveGroupsItem(uoLineSpan* peSpn);
		virtual bool saveGroupsHeaderEnd(uoRptHeaderType rht);

		virtual bool saveSectionHeaderStart(int count, uoRptHeaderType rht);
		virtual bool saveSectionItem(uoLineSpan* peSpn);
		virtual bool saveSectionHeaderEnd(uoRptHeaderType rht);

		virtual bool saveScaleHeaderStart(int count, uoRptHeaderType rht);
		virtual bool saveScaleItem(uoRptNumLine* rLine);
		virtual bool saveScaleHeaderEnd(uoRptHeaderType rht);

		virtual void saveRowsStart(int rowCount);
//		virtual void saveRowItemStart(int rowNumb, int cellCount);
		virtual void saveRowItemStart(uoRow* row);
		virtual void saveCell(uoCell* cellItem);
		virtual void saveRowItemEnd();
		virtual void saveRowsEnd();

        virtual void saveMatrixStart(uoCellMatrix* matrx){ Q_UNUSED(matrx); }
		virtual void saveMatrixRowStart(int row);
		virtual void saveMatrixRowEmd();
        virtual void saveMatrixEnd(uoCellMatrix* matrx){ Q_UNUSED(matrx); }

		virtual void saveFontStart(int count);
		virtual void saveFont(QFont* psFont, int nom);
		virtual void saveFontEnd();


		/// Запись подвальной части
		virtual bool saveDocEnd(uoReportDoc* doc);

		virtual bool load(uoReportDoc* doc);

	private:
		QTextStream _textStream;
		QFile 		_outFile;
		int m_rowNumber;
		int m_colNumber, m_lastcolNumber;
		QString		m_allTextTab; /// Скинем сюда сразу текст из ячеек, что-бы при одном обходе раздобыть текст.

};


} /// namespace uoReport

#endif // UOREPORTLOADER_H
