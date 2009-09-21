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

#define uoReportVersion "0.2"
/*
	Старые версии: 0.1
*/
#define rptSize qreal
#define rptSizeNull 0.0

#define TEXT_TR_POINT_ARR_SIZE 400
#define UNDO_COMMAND_MAX_COUNT 50



/**
	Размер крайней области объекта в которой курсор принимает
	вид "перетаскивателя" и при клике в которой начинается
	перетаскивание или изменение размера.
*/
#define UORPT_DRAG_AREA_SIZE 2.0
#define UORPT_STANDART_OFFSET_TEXT 2.0
#define UORPT_LENGTH_TEXT_H_SECTION 7
#define UORPT_VIEW_SCROLL_KOEFF 2

#define rptRect QRectF

/*
	(примечание)
	Scale - ячейка линейки,
	Cell - ячейка основного поля, \n чисто для изучения английского :)
*/

class uoReportManager;
struct uoEnumeratedItem;
struct uoLineSpan;
struct uoRptNumLine;
class uoHeaderScale;
class uoReportDocFontColl;
class uoSpanTree;
struct uorPagePrintSetings;
class uoReportDoc;
class uoReportCtrlMesFilter;
class uoReportCtrl;
class uoReportView;
class uoReportViewIteract;
class uoReportDrawHelper;
struct uoTextTrPointCash;
class uoReportDocBody;
template <typename T> class uoNumVector;
template <typename T> class uoCacheItemizer;
class uoReportLoader;
class uoReportLoaderXML;
class uoReportSelection;
class uoReportPreviewDlg;
struct uoRptGroupItem;
struct uoRptSectionItem;


struct uorTextDecorBase;
struct uorReportAreaBase;
struct uorReportViewArea;
class uorPropDlg;
class uoTextPropTab;
class uoTextLayotTab;
class uoTextFontPropTab;

class 	uoReportPropEditor;
struct 	uoTextBoundary;
struct 	uorTextDecor;
struct 	uorBorderPropBase;
class 	uoRowsDoc;
struct 	uoCell;
struct 	uoCellJoin;
class 	uoRow;
struct 	uoRUndoUnit;
struct 	uoRUndo01;
class 	uoReportUndo;


///\enum uoIntersectMode - варианты перечения отрезков
enum uoIntersectMode {
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

/// Тип спанов: секция или группа..
enum uoRptSpanType
{
	uoSpanType_None = 0
	, uoSpanType_Group
	, uoSpanType_Sections
};
enum uoRptHeaderType {
	  uorRhtUnknown = 0
	, uorRhtRowsHeader = 1
	, uorRhtColumnHeader = 2
};

enum uoSearchDirection {
	toUp		///< направление поиска вверх
	,toDown		///< направление поиска ВНИЗ
};

/// Тип направления...
enum uoSideType{
	uost_Unknown 	= 0
	, uost_Top 		= 1
	, uost_Right 	= 2
	, uost_Bottom 	= 3
	, uost_Left 	= 4

};
enum uoScaleSizePolicy
{
	uoSSP_fixed = 0 ///< расширения не происходит
	, uoSSP_minimum ///< текущий размер минимально возможный, т.е. ячейка может расширяться.
	, uoSSP_maximum ///< установленный размер является максимальным, т.е. ячейка будет стремиться сужаться.
};

///\enum uoRptStoreFormat - Формат сохранения отчета.
enum uoRptStoreFormat {
	  uoRsf_Unknown = 0
	, uoRsf_XML = 1
	, uoRsf_Binary = 2
	, uoRsf_HTML = 3
};
/**
	\enum uorFixationTypes - тип фиксации областей отчета при просмотре
*/
enum uorFixationTypes
{
	uorAF_None = 0 		///< нет фиксации
	,uorAF_Rows = 1		///< фиксированы только строки
	,uorAF_Cols			///< фиксированы только столбцы
	,uorAF_RowsAndCols	///< фиксированы и строки и столбцы
};
/**
	\enum uoUiCommand - перечень комманд, задействованных в интерфейсе пользователя
	Буду использовать для управления набором пиктограмм, пунктов меню и т.п.
*/
enum uoUiCommandRpt
{
	uoUCR_Unknown = 0

	// Типа как-бы общие
	, uoUCR_Cat 	///< Вырезать
	, uoUCR_Copy 	///< Скопировать
	, uoUCR_Paste 	///< Вставить

	// Блок для управления строками/столбцами
	, uoUCR_CaptionDisplay 	///< Отображать заголовки
	, uoUCR_Insert 	///< Вставить
	, uoUCR_Delete 	///< Удалить

	, uoUCR_GridDisplay 	///< Отображать сетку

	// Блок для управления группами
	, uoUCR_GroupDisplay 	///< Отображать
	, uoUCR_GroupJoin 		///< Вставить
	, uoUCR_GroupExclude 	///< Удалить

	// Блок для управления секциями
	, uoUCR_SectionDisplay 	///< Отображать
	, uoUCR_SectionInclude 	///< Включить в секцию
	, uoUCR_SectionExclude 	///< Исключить из секции
};

/**
	\enum uoRptDocSparesType - типы запасных частей документа.
	\brief - Перечисление пространственных частей документа/вьюва.
	spares - запчасти ; запасные части; spare parts, spares.

	Пока буду использовать для акселерации вычисления части документа по
	пространственным координатам.
	Vst = view Spares Type
*/
enum uorSparesType {
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

///\enum uorSelectionType типы выделения во вьюве.
enum uorSelectionType {
	  uoRst_Unknown 	= 0 ///< Ничего не выделено, 1 текущая ячейка.
	, uoRst_Document 	= 1	///< Выделен весь документ
	, uoRst_Column			///< Выделена колонка
	, uoRst_Columns			///< Выделены колонки
	, uoRst_Row				///< Выделена строка
	, uoRst_Rows			///< Выделены строки
	, uoRst_Cell			///< Выделена ячейка
	, uoRst_Cells			///< Выделены ячейки
	, uoRst_Mixed			///< Миксированное выделение(ячеек?).
};

///\enum uorBorderLocType - расположения бордюра
enum uorBorderLocType
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

/**
	\enum uoReportStateMode - режим взаимодействия с пользователем

	Например: редактирование ячейки, выделение групп ячеек, и т.п.
*/
enum uoReportStateMode {
	rmsNone = 0 			///< Обычный режим. Серфинг с пом. КПК по вьюву.
	, rmsResizeRule_Top  	///< Изменение размера вертикальной ячейки линейки.
	, rmsResizeRule_Left 	///< Изменение размера ГОРИЗОНТАЛЬНОЙ ячейки линейки.
	, rmsSelectionRule_Top  ///< Групповое выделения ячеек вертикальной линейки.
	, rmsSelectionRule_Left ///< Групповое выделения ячеек ГОРИЗОНТАЛЬНОЙ линейки.
	, rmsEditCell 			///< Редактирование текста ячейки в режиме инлайн.
	, rmsSelectionCell		///< Режим выделения ячеек мышкой или с пом КПК.
};

/**
	\enum uoCellTextBehavior - поведение текста при привышении его размера ширины ячейки.

	<pre>
	uoCTB_Auto - Текст будет печататься в сторону его выравнивания, пока не встретит
		либо границу документа, либо первую заполненную ячейку. Т.е. фактически текст
		может печататься поверх нескольких ячеек, если это ему нужно....
	uoCTB_Cut - встретив границу ячейки, текст просто обрежется.
	uoCTB_Obstruct - встретив границу ячейки, литеры текста поменяется на "@"
	uoCTB_Transfer - если текст не помещается в отведенную ему область, он будет
		перенесен и ячейка должна проинформировать строку, что её размер должен быть
		расширен.
	</pre>
*/
enum uoCellTextBehavior {
	  uoCTB_Unknown = 0 	///< Неопределенное,
	, uoCTB_Auto = 1
	, uoCTB_Cut	 = 2
	, uoCTB_Obstruct = 3
	, uoCTB_Transfer = 4
};
/*
	Неопределенные значения для свойств необходимы из-за групповой обработки ячеек..
	используется для!! группы выдененных ячеек, когда у них смешанные значения.
*/

///\enum uoVertAlignment - типы вертикального выравнивания
enum uoVertAlignment {
	  uoVA_Unknown 	= 0 ///< Неопределенное,
	, uoVA_Top 		= 1	///< Выравнивание по верхнему краю
	, uoVA_Center	= 2	///< Выравнивание по центру
	, uoVA_Bottom 	= 3	///< Выравнивание по нижнему краю
};

///\enum uoHorAlignment - типы горизонтального выравнивания
enum uoHorAlignment {
	  uoHA_Unknown 	= 0 ///< Неопределенное,
	, uoHA_Left 	= 1	///< Выравнивание по левому краю
	, uoHA_Center 	= 2 ///< Выравнивание по центру
	, uoHA_Right 	= 3 ///< Выравнивание по правому краю
};

/**
	\enum uoCellTextType - типы текста в ячейке
	\brief Типы текста в ячейке: текст, выражение, шаблон.

	<pre>
	С "текстом" все просто, его надо просто вывести
	"Выражение", достаточно просто, например "PrnPrice", просто
	ищем в списке выражений в документе и заменяем на значения
	Шаблон, это просто текст, который надо распарсить. Образец:
	"Общая сумма: [ВсегоСумма] руб.".
	Извлекаются части:
	- "Общая сумма: "
	- "[ВсегоСумма]"
	- " руб."
	"[ВсегоСумма]" - заменяется по принцыпу как и с "Выражением."
	Потом склеивается.
	</pre>
*/
enum uoCellTextType {
	 uoCTT_Unknown = 0	///< Простой текст, не интерпретируемый
	, uoCTT_Text = 1	///< Простой текст, не интерпретируемый
	, uoCTT_Expr 	///< Выражение, должно быть взято из хранилища выражения
	, uoCTT_Templ 	///< Шаблон. должен быть разобран и интерпретирован.
};


/**
	\enum uoCellBorderType - тип линии бордюра, просто повторение Qt::PenStyle

	Тут бы надо подумать над стилями:
	uoCBT_SolidSolidLine - 2 линии рядышком...
	и виджет для редактирования
*/
enum uoCellBorderType {
	  uoCBT_Unknown = -1
	, uoCBT_NoPen = 0
	, uoCBT_SolidLine = 1
	, uoCBT_DashLine
	, uoCBT_DotLine
	, uoCBT_DashDotLine
	, uoCBT_DashDotDotLine
	, uoCBT_CustomDashLine // - оно мне надо? :)
};
/**
	\enum uoCellsJoinType типы объединений ячеек.
	<pre>
	<b> uoCJT_BackPoint </b> - Указатель назад: необходим в объединенных ячейках указатель на стартовую. Вот это он и есть.
	т.е. значения (m_Coord1 и m_Coord2) означают:
		m_row - порядковый номер строки этой ячейки в объдинении;
		m_col - порядковый номер столбца этой ячейки в объдинении.

	<b>uoCJT_Formal</b> - Формальное объединение, означает что ячейки НЕ ОБЪЕДИНЕНЫ, но текст выравнивается
	т.е. значения (m_Coord1 и m_Coord2) означают:
		m_row = количество строк в объединении,
		m_col = количество столбцов в объединении

	<b>uoCJT_Normal</b> -Нормальное объединение, означает что ячейки ОБЪЕДИНЕНЫ
	т.е. значения (m_Coord1 и m_Coord2) означают:
		m_row = количество строк в объединении,
		m_col = количество столбцов в объединении

	пример:
           1  2  3  4  5  6  7
        1 [ ][ ][ ][ ][ ][ ][ ]
        2 [ ][ ][ ][ ][ ][ ][ ]
        3 [ ][ ][*][*][ ][ ][ ]
        4 [ ][ ][*][*][ ][ ][ ]
        5 [ ][ ][*][*][ ][ ][ ]
        6 [ ][ ][ ][ ][ ][ ][ ]
        7 [ ][ ][ ][ ][ ][ ][ ]

		Объединены ячейки R3C3:R5C4
		в ячейке R3C3
		uoCellJoin->m_JoinType = uoCJT_Normal;
		uoCellJoin->m_row = 3;
		uoCellJoin->m_col = 2;

		в ячейке R3C4
		uoCellJoin->m_JoinType = uoCJT_BackPoint;
		uoCellJoin->m_row = 1;
		uoCellJoin->m_col = 2;

		в ячейке R5C4
		uoCellJoin->m_JoinType = uoCJT_BackPoint;
		uoCellJoin->m_row = 3;
		uoCellJoin->m_col = 2;
		примечание: нумерация начинается с 1 (с единицы).
		</pre>
*/
enum uoCellsJoinType{
	  uoCJT_Unknown = 0 // Нет объединения
	, uoCJT_BackPoint 	// Абстрактное объединение
	, uoCJT_Normal 		// Нормальное объединение
	, uoCJT_Formal 		// Формальное объединение
};

/**
	Типы поиска в uoSpanTree.
*/
typedef enum uoSTScanType
{
	uoSTST_Unknown = 0
	, uoSTST_ById = 1
	, uoSTST_ByName = 2
};

/**
	\enum uoDocChangeType - типы изменений документа.
	\brief Используется для типизации изменеия документа для механизма Undo/Redo
*/
typedef enum uoDocChangeType
{
	uoDCT_Unknown = 0
	, uoDCT_ClearDoc  	/// очистка документа.
	, uoDCT_ChageText  	/// Изменение текста ячейки...
};
/**
	\enum uorPropertyTabType - типы закладок плавающей панели свойств.
	\brief типы закладок плавающей панели свойств.
*/

typedef enum uorPropertyTabType
{
	  uorPropTTab_Unknown = 0
	, uorPropTTab_Text = 1
	, uorPropTTab_TextLayot = 2
	, uorPropTTab_TextFont = 3
	, uorPropTTab_Border = 4
};
/**
	\enum uorSelVisitorType - типы процессора обработки выделения.
	\brief Используется для идентификации визитера.
*/
typedef enum uorSelVisitorType
{
	 uorSVT_Unknown = 0
	, uorSVT_Getter = 1
	, uorSVT_Setter = 2
};

/**
	\enum uorHeaderScaleChangeType - типы изменения линейки отчета.
	\brief Используется для механизма Ундо/Редо
*/
typedef enum uorHeaderScaleChangeType
{
	 uorHSCT_Unknown = 0
	, uorHSCT_Hide = 1
	, uorHSCT_Size = 2
	, uorHSCT_Fixed = 3
};
/**
	\enum uorRCGroupOperationType - идентификаторы групповых операций над строками/столбцами.
*/

typedef enum uorRCGroupOperationType
{
	 uorRCGroupOT_Unknown = -1
	, uorRCGroupOT_Delete = 1
	, uorRCGroupOT_Add = 2
	, uorRCGroupOT_SetSize = 3
	, uorRCGroupOT_SetAutoSize = 4
};


/// Запуск тестов для элементов отчета...
extern void uoRunTest();

class uoReportTest {
	public:
		uoReportTest();
		~uoReportTest();
		void exploreQPrinter();
};

} // namespace uoReport


#endif // UOREPORTCOMMON_H_INCLUDED
