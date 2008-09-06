#ifndef UOREPORTCOMMON_H_INCLUDED
#define UOREPORTCOMMON_H_INCLUDED

/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

namespace uoReport {

//#include "uoReportDescr.h" // убрал, потому что доксиген дублирует описание в \subpage

// На глазок вроде ничего..
#define UORPT_SCALE_SIZE_DEF_VERTICAL 	15.0
#define UORPT_SCALE_SIZE_DEF_HORIZONTAL 60.0

#define uoReportVersion "0.1"
#define rptSize qreal
#define rptSizeNull 0.0

/**
	Размер крайней области объекта в которой курсор принимает
	вид "перетаскивателя" и при клике в которой начинается
	перетаскивание или изменение размера.
*/
#define UORPT_DRAG_AREA_SIZE 2.0

#define rptRect QRectF

/*
	(примечание)
	Scale - ячейка линейки,
	Cell - ячейка основного поля, \n чисто для изучения английского :)
*/



struct uoLineSpan;
struct uoRptNumLine;
class uoSpanTree;
class uoReportDoc;
class uoReportCtrl;
class uoReportView;
class uoReportViewIteract;
class uoReportDocBody;
template <typename T> class uoNumVector;
class uoReportLoader;
class uoReportLoaderXML;
class uoReportSelection;


///\enum uoIntersectMode - варианты перечения отрезков
typedef enum uoIntersectMode {
	  ismNone 		= 0
	, ismAbove 		= 1
	, ismAboveIn 	= 3
	, ismIn 		= 2
	, ismInBelow 	= 6
	, ismBelow 		= 4
	, ismSurround 	= 7
	, ismAboveUp 	= 9
	, ismBelowDown 	= 12
};
typedef enum uoRptHeaderType {
	  rhtUnknown = 0
	, rhtVertical = 1
	, rhtHorizontal = 2
};

typedef enum uoSearchDirection {
	toUp		///< направление поиска вверх
	,toDown		///< направление поиска ВНИЗ
};

/// Тип направления...
typedef enum uoSideType{
	uost_Unknown 	= 0
	, uost_Top 		= 1
	, uost_Right 	= 2
	, uost_Bottom 	= 3
	, uost_Left 	= 4

};
typedef enum uoScaleSizePolicy
{
	uoSSP_fixed = 0 ///< расширения не происходит
	, uoSSP_minimum ///< текущий размер минимально возможный, т.е. ячейка может расширяться.
	, uoSSP_maximum ///< установленный размер является максимальным, т.е. ячейка будет стремиться сужаться.
};

///\enum uoRptStoreFormat - Формат сохранения отчета.
typedef enum uoRptStoreFormat {
	  uoRsf_Unknown = 0
	, uoRsf_XML = 1
	, uoRsf_Binary = 2
	, uoRsf_HTML = 3
};

/**
	\enum uoRptDocSparesType - типы запасных частей документа.
	\brief - Перечисление пространственных частей документа/вьюва.
	spares - запчасти ; запасные части; spare parts, spares.

	Пока буду использовать для акселерации вычисления части документа по
	пространственным координатам.
	Vst = view Spares Type
*/
typedef enum uoRptSparesType {
	  uoVst_Unknown = 0
	, uoVst_GroupV = 1	///< рект вертикальной группы
	, uoVst_GroupH		///< рект ГОРИЗОНтальной группы
	, uoVst_ScaleVH 	///< Прямоугольник для выделения документа.
	, uoVst_ScaleV 		///< Ячейка вертикальной линейки.
	, uoVst_ScaleH 		///< Ячейка ГОРИЗОНтальной линейки.
	, uoVst_SectionV 	///< Вертикальная секция.
	, uoVst_SectionH 	///< Горизонтальная секция.
	, uoVst_Cell	 	///< Ячейка таблицы или объединение.
};	// Vst - (ViewSparesType)

///\enum uoRptSelectionType типы выделения во вьюве.
typedef enum uoRptSelectionType {
	  uoRst_Unknown 	= 0
	, uoRst_Document 	= 1	///< Выделен весь документ
	, uoRst_Column			///< Выделена колонка
	, uoRst_Columns			///< Выделены колонки
	, uoRst_Row				///< Выделена строка
	, uoRst_Rows			///< Выделены строки
	, uoRst_Cell			///< Выделена ячейка
	, uoRst_Cells			///< Выделены ячейки
	, uoRst_Mixed			///< Миксированное выделение.
};

///\enum uoBorderLocType - расположения бордюра
typedef enum uoBorderLocType
{
	uoBlt_Unknown = 0 /// неопознаный %)
	, uoBlt_Top  = 1 ///< верхний бордер
	, uoBlt_Right	 ///< правый бордер
	, uoBlt_Bottom	 ///< нижний бордер
	, uoBlt_Left  	 ///< левый бордер
	///Отсчет по часовой.

};

///\enum uoReportUseMode - режим работы с отчетом разработка или использование
enum uoReportUseMode {
	/**
		Режим разработки.
		По умолчанию включает видимости секций, линейки, сетки.
		Все остальное отключено. Не испускаетют сигналы "приКликеНаЯчейке"
	*/
	rmDevelMode = 0
	/**
		Режим отображения.
		По умолчанию включает видимости группировок. Все остальное отключено.
		Сигналы "приКликеНаЯчейке" испускаются со значением расшифровки.
	*/
	, rmUsingMode = 1
};

///\enum uoReportStateMode - режим взаимодействия с пользователем
/// Например: редактирование ячейки, выделение групп ячеек, и т.п.
enum uoReportStateMode {
	rmsNone = 0 			///< Обычный режим. Серфинг с пом. КПК по вьюву.
	, rmsResizeRule_Top  	///< Изменение размера вертикальной ячейки линейки.
	, rmsResizeRule_Left 	///< Изменение размера ГОРИЗОНТАЛЬНОЙ ячейки линейки.
	, rmsSelectionRule_Top  ///< Групповое выделения ячеек вертикальной линейки.
	, rmsSelectionRule_Left ///< Групповое выделения ячеек ГОРИЗОНТАЛЬНОЙ линейки.
};


/// Запуск тестов для элементов отчета...
extern void uoRunTest();

class uoReportTest {
	uoReportTest();
	~uoReportTest();
};

} // namespace uoReport


#endif // UOREPORTCOMMON_H_INCLUDED
