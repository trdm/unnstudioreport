/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************

    by BaryVetaL, as Barilko V.V. 2008

**************************************/

#ifndef UOREPORTBKIMAGE_H_INCLUDED
#define UOREPORTBKIMAGE_H_INCLUDED


#include <QObject>
#include <QPainter>
#include <QImage>
#include <QPointF>

namespace uoReport {

/**
	\class uoReportBkImage - подложка (фоновое изображение)
	\brief Класс реализация подложки в uoReportCtrl. Занимается отрисовкой
        подложки на виджите uoReportCtrl позволяет масштабировать
        изображение подложки

		uoReportBkImage - класс подложка
*/
// todo при вставке возможности
///\todo в класс uoReportCtrl добавить uoReportBkImage _bkImage; /// << Хранит подложку документа
///\todo в uoReportCtrl.clear() добавить _bkImage.clear();
///\todo добавить событие в onSetScaleFactor чтобы там вызывалась recalcbkImage(_scaleFactor);
///\todo в drawDataArea добавить _bkImage.drawBkImage(painter);
///\todo вообщем то все используй в uoReportCtrl как хочешь :)
///\todo При загрузке необходимо совершить следующее
/// if(!loadBkImage(fileName)) {
///     recalcbkImage(scaleFactor);
///     emit update();
/// }


///\todo Добавить проверку, что при скролинге подложка попадает в область

class uoReportBkImage : public QObject
{
    Q_OBJECT
    public:
 		uoReportBkImage(){
            _bkImage = NULL;
            _bkImageOriginal = NULL;
		}

		~uoReportBkImage(){
            clear();
		}
        /// Отрисовка подложки в painter в точке (startx, starty)
        void drawBkImage(QPainter& painter){
            // Отрисовка _bkImage (подложки) ИДЕТ ПЕРВОЙ до отрисовки вертикальных и горизонтальных линий!
            // При условии конечно, что _bkImage загружен
            if (_bkImage != NULL) {
                painter.drawImage(QPoint(int(startPoint.x()), int(startPoint.y())), *_bkImage);
            }
            // конец отрисовки подложки
        }

        /// Увеличить/уменьшить изображение с учетом масштаба
        /// scaleFactor - величина коэффициента увеличения/уменьшения
        void recalcbkImage(qreal scaleFactor){
            if (_bkImageOriginal != NULL){
                *_bkImage = _bkImageOriginal->copy();
                int x = (int)(_bkImage->width() * scaleFactor);
                int y = (int)(_bkImage->height() * scaleFactor);
                _bkImage->scaled(QSize(x, y), Qt::IgnoreAspectRatio);
            }
        }

        /// Загрузить подложку из файла fileName
        /// а в отрисовке drawDataArea() вызывать drawBkImage(painter, startx, starty)
        bool loadBkImage(QString fileName){
            if (!fileName.isEmpty()){
                _bkImageOriginal = new QImage();

                if (!_bkImageOriginal->load(fileName)) {
                    delete _bkImageOriginal;
                    _bkImageOriginal = NULL;
                    return false;
                }
                else {
                    _bkImage = new QImage();
                    return true;
                }
            }
        }

        /// Очистить подложку
        void clear(){
            // Если есть что удалить удаляем
            if (_bkImageOriginal != NULL) {
                delete _bkImage;
                delete _bkImageOriginal;
            }
            _bkImage = NULL;
            _bkImageOriginal = NULL;
        }

        /// Проверить подложка пуста
        bool isEmpty(){
            return _bkImageOriginal = NULL;
        }

        /// точка с координатами смещения
        QPointF startPoint;
    private:
        // Уменьшенное/увеличенное изображение
        QImage* _bkImage;
        // Изображение оригинал
        QImage* _bkImageOriginal;
}; //namespace uoReport

}

#endif // UOREPORTBKIMAGE_H_INCLUDED
