/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOREPORTDOC_H
#define UOREPORTDOC_H

#include <QObject>
#include <QLinkedList>
#include <QList>
#include "uoReport.h"
#include "uoSpanTree.h"
#include "uoReportDocBody.h"


namespace uoReport {


///\class uoReportDoc - обслуживает данные таблицы отчета
///\brief обслуживает данные таблицы отчета.
class uoReportDoc
	: public QObject
	, public uoReportDocBody
{
    Q_OBJECT
	public:
		uoReportDoc();
		virtual ~uoReportDoc();
	public:
		void clear();
		bool addGroup(int start, int end, uoRptHeaderType ht);
		bool addSection(int start, int end, uoRptHeaderType ht);

		bool possiblyAddGroup(int start, int end, uoRptHeaderType ht);
		bool possiblyAddSection(int start, int end, uoRptHeaderType ht);

		void onDeleteLine(int lineStart, int count = 1);
		const spanList* getGroupList(uoRptHeaderType rht, int start, int end);

		int getGroupLevel(uoRptHeaderType ht);
		int getSectionLevel(uoRptHeaderType ht);

		void doGroupFold(int idGrop, uoRptHeaderType rht, bool expand);

		bool save();
		bool saveToFile(QString path, uoRptStoreFormat stFormat);
		bool saveOptionsIsValid();
		bool flush(uoReportLoader* loader);


		uoRptStoreFormat 	getStoreFormat(); ///< Возвращает установленный формат сохранения
		QString 			getStorePathFile();///< Возвращает установленное имя файла.
		void 				setStoreOptions(QString  filePath, uoRptStoreFormat stFormat); ///< установим опции сохранения

	protected:
	private:

	protected:
		int _maxLevelSpanFoldingH;   	///< Группировки
		int _maxLevelSpanSectionsH;		///< секции.

		QString 		 _docFilePath;		///< Имя файла
		uoRptStoreFormat _storeFormat;	///< Формат хранения файла отчета.

		uoSpanTree* _spanTreeGrH;
		uoSpanTree* _spanTreeGrV;
		uoSpanTree* _spanTreeSctH;
		uoSpanTree* _spanTreeSctV;
};

} // namespace uoReport

#endif // UOREPORTDOC_H
