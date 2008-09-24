#ifndef UOREPORTDESCR_H_INCLUDED
#define UOREPORTDESCR_H_INCLUDED

/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
namespace uoReport{

/**
	\mainpage uoReport - unNStudio-Report
	<b>unNStudio - un named studio (не названная студия).</b> \n
	Компонент для подготовки и рендринга печатныз форм. \n
	Разрабатывается как отдельный компонент, который можно \n
	использовать отдельно от unNStudio и используется как полигон \n
	для обкатки решений. \n

	Дополнитльная информация: \n
	- \subpage unNStudio "unNStudio - un named studio (не названная студия)"
	- \subpage uoReportDoc 	"Табличный документ"
	- \subpage uoReportCtrl "Табличный редактор"
	- \subpage uoReportIdeas "Идеи и хотелки под табличный редактор"
*/

//-----------------------------------------------------------

/*! \page unNStudio unNStudio
	<b>unNStudio - un named studio (не названная студия).</b> \n
	(никак не могли договориться с коллективом о названии, отсюда некое компромисное решение)  \n
	\n
	Краткое описание проекта <b>unNStudio</b>, это синтез:  \n

		- 1С_77(скорость/простота в конфигурировании);  \n
		- 1С++:  \n
		- ООП в разумных контролируемых/реализуемых пределах, хочется сделать уникальную систему \n
		наследования объектов, а то достало один и тот-же код писать в разных местах. \n
		- табличные поля \n
		- 1С_8: НЕ ВСЕ, но частности: гибкие блокировки, множественные табличные части \n
		документов, вынесение периодики в регистры сведений. \n
		пока хватит, тащить все из v8 не собираемся ни в коем случае! \n
	\n
		-------------------------------------------------- \n
		Основные направления планируемого развития: \n
		- Все достаточно просто и сложно одновременно. \n
		Камешек преткновения интерпретатор будет делаться на первом этапе, \n
		осилим его, проекту быть. Я его считаю фундаментальным компонентом, \n
		 а гуй и формочки для обслуживания метаданных можем потом нарисовать. \n
		Объекты метаданных будут развиваться поэтапно. \n
		Конфигуратор и среда исполнения будут развиваться паралельно. \n
		Первый этап интерпретатор, свой дизайнер форм, константы, справочники. \n
		Второй этап система печати, перечисления, меню, панели инструментов. \n
		пока хватит? \n
		Задача такая: результат каждого этапа есть цельноработающее приложение, пригодное для \n
		элементарного использовния. Никаких там: гоним конфигуратор, а потом интерпрайз мне и \n
		нафиг не надо. А вам? \n
*/


/**
	\page uoReportDoc "Табличный документ"

	Содержимое тела документа: строки, текст, картинки и т.п. \n
		Документ представляет собой таблицу, являющую собой \n
	пересечение строк и столбцов. Стобцы и строки имеют определенный \n
	размер, признак выделения, признак выбранности, номер, а так же \n
	некоторые дополнительные признаки, которые выявятся по ходу \n
	необходимости.
		\n
	Ячейка содержит данные: текст, расшифровку \n
	и примечание. Текст служит для отображения его на экране и принтере. \n
	Примечание является просто формой подсказки. Расшифровка, это элемент скрытого \n
	механизма, позволяющего взаимодействовать с отчетом в режиме вывода + только \n
	чтения. В данном режиме при двойном щелчке на ячейке расшифровки или нажатии \n
	клавиши Enter отчет посылает сигнал, содержащий расшифровку.
		\n
		Табличный редактор может иметь разную ширину ячеек в пределах отной строки. \n
	Это было сделано для облегчения макетирования сложных несиметричных печатных форм. \n
		\n
		Табличный редатор поддерживает режимы редактирования и только чтения. \n

		Табличный документ может содержать в себе не только отформатированый текст, но \n
	и поддерживает несколько графических примитивов для обогащения визуального  \n
	оформления отчетов. В списке примитивов планируется: \n
		- Линия. \n
		- Прямоугольник с текстом; \n
		- Круг. \n
		- Картинка. \n
		- SVG виджет. \n
	\n
	Проблема дефолтности
	или какие свойства хранить в каких объектах \n
	 \n
	<b> Документ, строка или столбец хранит: </b> \n
		- шрифт: цвет, размер, семейство в форме ID, стиль.  \n
		- фон: цвет фона. \n
	при установке  одного из этих показателей во всем документе очищаются эти проперти. \n
		- <i> список шрифтов: семейство в форме ID и строки.</i> \n
	<b> Ячейка хранит: </b> \n
		- шрифт: цвет, размер, семейство в форме ID, стиль.  \n
		- тип бордюра для каждой стороны.  \n
		- цвет бордюра общий для всех сторон.  \n


*/


/**
	\page uoReportCtrl "Табличный редактор"

		Табличный редактор представляет собой разлинованную таблицу состоящую из \n
	неограниченного количества нумерованных строк и столбцов. На пересечении \n
	строки и столбца находится ячейка. \n
	Оформление ячейки.	\n
	\n
	<b>Логика работы</b> \n
	- Свертка/развертка группировок строк/столбцов, логика работы. \n
	\n
	Свертка группы предполагает установку неидимости на строки/столбцы. \n
	Достаточно при свертке установить видимым столбцам признак невидимости. \n
	Развертка - обратное действие, устанавливаем видимость невидимым столбцам. \n
	И в том и в другом процессе не трогаем свернутые внутри группы строки/столбцы.  \n
	\n
	- Выделения и типы выделений. \n
	\n
	Выделения - выбранные части документа: все документ, строки, столбцы, области ячеек  \n
	отдельные ячейки, объединения ячеек. Выделение объектов служит для групповых операций \n
	с ними. К примеру необходимо сгруппировать серию рядом лежащих строк или столбцов. \n
	Установить шрифт или фон определенной группе ячеек. \n
	\n
	Работа по выделениею объектов в поле данных производится как мышкой, так и клавишами  \n
	перемещения курсора. Если необходимо выделить строки или столбцы, на которых стоит курсор  \n
	необходимо воспользоваться мышкой или комбинацией клавиш Ctrl+Space для выделения  \n
	строки/строк и Shift+Space для выделения колонок. \n
	Если надо добавить выделяния к тем что уже есть, то необходимо нажать клавишу Ctrl \n
	Особенности. Мы можем одновременно выделить только (или/или):	\n
		- Диапазон ячеек;	\n
		- Весь документ;	\n
		- Строки;	\n
		- Столбцы;	\n

	- Скролинг документа. \n
		У документа есть несколько видов размеров - реальные и виртуальные. \n
		И те и дугие имеют несколько измирителей:	\n
			- количество строк и столбцов \n
			- длина и высота в "точках"	 см. литературу по dpi - точки на дюйм. \n
		Смысл разделения на такие категории облегчить редактирование документа 	\n
		с помощью скоринга. Надо хорошо подумать над скролингом.	\n
		 \n
		 \n
		 \n

*/

/**
	\page uoReportIdeas "Идеи и хотелки под табличный редактор"

	- Добавить натуральную линейку для точного расчета местоположения встраиваемых \n
	объектов!!! кул идейка!!!! \n
	- Реализовать разную ширину ячеек в пределах 1 колонки. \n
	- Сделать режим "подложки". Пояснения: часто в моей работе требовалось реализовать \n
		точную печать на каком нибудь бланке формата А4. Очень долго трахался с 1С-ным    \n
		табличным редактором, целясь так, что-бы каждый квадратик для "галочки"	\n
		располагался милиметр в милиметр там где он нужен.	Печатая десятки копий \n
		для пробы. \n
		идея очень проста: сканируется анкета, подкладывается под "поле данных"	\n
		выравнивается с помощью смещения верхнего левого угла и масштабирования	\n
		и пары тестовых размещений элементов печати и начинается макетирование 	\n
		самой печатной формы под анкету.	\n

*/

} //namespace uoReport

#endif // UOREPORTDESCR_H_INCLUDED
