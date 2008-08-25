#ifndef UONUMVECTOR_H_INCLUDED
#define UONUMVECTOR_H_INCLUDED

/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include <QLinkedList>
#include "uoReport.h"

namespace uoReport {

///\class uoNumVector Шаблонный расширяемый класс для хранение и обработки длинных нумерованных структур.
///\brief Шаблонный расширяемый класс для хранение и обработки длинных нумерованных структур.
///
/// Определены стандартные операции установки размера, запроса размера, чистка списка, \n
/// удаление диапазонов итемов. Все остальное нужно определить в производных классах.
template <typename T>
class uoNumVector
{
 	protected:
		QLinkedList<T*>* _list;
		typename QLinkedList<T*>::iterator _itSave;
		bool _itSaveOnUse; ///< итератор _itSave - используется.
		int _maxNo, _minNo;
		rptSize _def_size;

	public:
		uoNumVector()
			: _itSaveOnUse(false)	{
			_list = new QLinkedList<T*>;
			_def_size = -1;
			_maxNo = _minNo = 0;
		};
		~uoNumVector()	{
			_itSave = NULL;
			if (_list->size() == 0)
				return;
			clear();
		}
		/// Определить Min и Max № после вставки итема в список.
		void defineMinMax(int nom) {
			if (_minNo == 0 && _maxNo == 0) {
				_minNo = _maxNo = nom;
			} else {
				if (_minNo>nom)	_minNo = nom;
				if (_maxNo<nom)	_maxNo = nom;
			}
		}

		/// Очистка вектора
		void clear() {
			detachIter();
			T* item = NULL;
			typename QLinkedList<T*>::iterator it = _list->begin();
			while (it != _list->end() ) {
				item = *it;
				it = _list->erase(it);
				item->~T();
				delete item;
				item = NULL;
			}
		}

		/// Функция создания нового итема. Возможно пригодится для ундо/редо.
		void onDeleteItem(T* delItem){}



		/// удаление итема/итемов
		void deleteItem(int nom, int cnt = 1)
		{
			if (cnt<=0)
				return;
			T* item = NULL;

			int cntDelItem = 0, lastNo = 0;
			for (int i = 0; i<cnt; i++) {
				lastNo = nom + i;
				if (findItem(lastNo, false)) {

					item = *_itSave;
					onDeleteItem(item);
					item = NULL;
					_itSave = _list->erase(_itSave);
					++cntDelItem;
				}
			}

			detachIter();

			/*
				Нужно сдвинуть номера итемов вверх на количество удаленных,
				начиная с последнего удаленного итема или с того номера,
				который должен быть удален.
			*/

			typename QLinkedList<T*>::iterator it = _list->begin();
			item = *it;
			_minNo = item->number();
			while (it != _list->end() ) {
				item = *it;
				_maxNo = item->number();
				if (_maxNo > lastNo) {
					_maxNo = _maxNo - cnt;
					item->setNumber(_maxNo);
				}
				it++;
			}
		}

		/// Получение размера списка
		int getCountItem() {
			return _list->size();
		}

	protected:

		/// Отключаемся от итератора.
		void detachIter() {
			_itSave	= NULL;
			_itSaveOnUse = false;
		}


		/// Функция создания нового итема
		T* createItem(int non) {
			T* item = new T(non);
			item->setSize(_def_size);
			return item;
		}

		///\todo отработать вставку/поиск до нормального уровня.
		/// Поиск итема по номеру в списке. При необходимости происходит создание итема. \n Просто позиционируемся на нужном итераторе
		bool findItem(int nom, bool needCreate = false)
		{
			T* item = NULL;
			if (_list->isEmpty()){
				if (needCreate) {
					item = createItem(nom);
					_list->append(item);
					_itSave = _list->begin();
					_itSaveOnUse = true;
					defineMinMax(nom);
					return true;
				}
				return false;
			}


			int reserchCnt = 0;

			reserch:
			++reserchCnt;
			if (reserchCnt>2)
				return false;

			if (!_itSaveOnUse) {
				_itSave = _list->begin();
				_itSaveOnUse = true;
			}

			item = *_itSave;

			uoSearchDirection srchDirection = toUp;
			int lastItemNo = 0;
			int itemNo = item->number();

			if (itemNo == nom)
				return true;
			else if (itemNo > nom) {
				srchDirection = toUp;
				_itSaveOnUse = false;
				while (_itSave != _list->begin()){
					item = *_itSave;
					lastItemNo = item->number();
					if (lastItemNo == nom)
						return true;
					else if (lastItemNo < nom) {
						// Поймали позицию итератора, после которой можно вставить итем.
						_itSaveOnUse = true;
						break;
					}
					_itSave--;
				}
			} else if (itemNo < nom) {
				srchDirection = toDown;
				_itSaveOnUse = false;
				while (_itSave != _list->end() ){
					item = *_itSave;
					lastItemNo = item->number();
					if (lastItemNo == nom)
						return true;
					else if (lastItemNo > nom) {
						_itSaveOnUse = true;
						break;
					}
					_itSave++;
				}
			}

			// Сюда доходим если не найшли итем.
			if (needCreate) {
				item = createItem(nom);
				if (addItem(item)) {
					defineMinMax(nom);
					_itSaveOnUse = false;
					goto reserch;
				} else {
					return false;
				}
				_itSaveOnUse = false;
				goto reserch;
			}
			return false;
		}

		/// Добавим итем в список, для надежности прийдется пробежаться по нему
		bool addItem(T* psItem){
			int itemNo = psItem->number();
			if ( itemNo < _minNo && _minNo>0) {
				_list->prepend(psItem);
				return true;
			} else if (itemNo > _maxNo && _maxNo > 0) {
				_list->append(psItem);
				return true;
			} else {
				int itemNo2 = psItem->number();
				T* item = NULL;
				typename QLinkedList<T*>::iterator it = _list->begin();
				while (it != _list->end() ) {
					item = *it;
					itemNo2 = item->number();
					if (itemNo2 > itemNo) {
						item->setNumber(_maxNo);
						it = _list->insert(it, psItem);
						return true;
					}
					it++;
				}
			}
			return false;
		}
	public:
 		/// Получить размер итема, а если итем не существует, тогда его дефолтный размер..
		rptSize getSize(int nom, bool isDef = false){
			if (findItem(nom)) {
				T* item = *_itSave;
				if (item)
					return item->size(isDef);
			}
			return _def_size;
		}

		/// Установить размер итема, а если итем не существует, создать и установить
		bool	setSize(int nom, rptSize size, bool isDef = false){
			if (!findItem(nom, true)){
				return false;
			} else {
				T* item = *_itSave;
				if (item) {
					item->setSize(size, isDef);
					return true;
				}
			}
			return false;
		}


		/// Установить дефолтный размер итема
		void setDefSize(rptSize size) {		_def_size = size;	}
		/// Получить дефолтный размер итема.
		rptSize getDefSizeItem() {	return _def_size;	}


};


} //namespace uoReport


#endif // UONUMVECTOR_H_INCLUDED
