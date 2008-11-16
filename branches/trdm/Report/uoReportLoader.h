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
#include <QTextStream>
#include <QTextDocument>
#include <QDataStream>
#include <QFile>
#include <QtXml>
//#include <QDomNode>



namespace uoReport {
// Loader - 1) грузчик
class uoReportLoader
{
	public:
		virtual ~uoReportLoader();

		static uoReportLoader* getLoader(uoRptStoreFormat stFormat);

		void 			setFileName(QString fileName);
		void 			setFormat(uoRptStoreFormat stFormat) {_storeFormat = stFormat;};
		QString				getFileName() {return _docFilePath;};
		uoRptStoreFormat	getFormat() {return _storeFormat;};

		QString getLastError() {return _lastError;};
		void 	setLastError(QString errMes) {_lastError = errMes;};


		bool validateStoreProps(bool forLoad = true);

		virtual bool init(bool forLoad = true) = 0; ///<Инициализация лоадера.
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
		virtual void saveRowItemStart(int rowNumb, int cellCount) = 0;

		virtual void saveCell(uoCell* cellItem) = 0;

		virtual void saveRowItemEnd() = 0;
		virtual void saveRowsEnd() = 0;

		virtual void saveFontStart(int count) = 0;
		virtual void saveFont(QFont* psFont, int nom) = 0;
		virtual void saveFontEnd() = 0;

		/// Запись подвальной части
		virtual bool saveDocEnd(uoReportDoc* doc) = 0;

		///================================================
		///===============LOAD=============================
		virtual bool load(uoReportDoc* doc) = 0;
		// остальное помоему можно пока и в производном классе сделать... пока...

		///===============LOAD=============================
		///================================================


	private:
		QString 		 _lastError;	///< Имя файла
		QString 		 _docFilePath;	///< Имя файла
		uoRptStoreFormat _storeFormat;	///< Формат хранения файла отчета.

};

class uoReportLoaderXML : public uoReportLoader {
	public:
		uoReportLoaderXML();
		virtual ~uoReportLoaderXML(){};

		virtual bool init(bool forLoad = true);
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
		virtual void saveRowItemStart(int rowNumb, int cellCount);
		virtual void saveCell(uoCell* cellItem);
		virtual void saveRowItemEnd();
		virtual void saveRowsEnd();

		virtual void saveFontStart(int count);
		virtual void saveFont(QFont* psFont, int nom);
		virtual void saveFontEnd();


		/// Запись подвальной части
		virtual bool saveDocEnd(uoReportDoc* doc);

		virtual bool load(uoReportDoc* doc);

		bool loadGroupsHeader(const QDomElement &node, uoReportDoc* doc);
		bool loadSectionHeader(const QDomElement &node, uoReportDoc* doc);
		bool loadScalesHeader(const QDomElement &node, uoReportDoc* doc);
		bool loadRows(const QDomElement &node, uoReportDoc* doc);
		bool loadCell(const QDomElement &node, uoReportDoc* doc, int rowNum);
		bool loadFont(const QDomElement &node, uoReportDoc* doc);


	private:
		QTextStream _textStream;
		QFile 		_outFile;


};


} /// namespace uoReport

#endif // UOREPORTLOADER_H
