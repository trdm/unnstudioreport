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

	public:
		uoNumVector()
			: _itSaveOnUse(false)	{
			_list = new QLinkedList<T*>;
			_maxNo = _minNo = 0;
		};
		virtual ~uoNumVector()	{
			_itSave = NULL;
			if (_list->size() == 0){
				delete _list;
				return;
			}
			clear();
			delete _list;
		}

		int getMinNo(){			return _minNo;		}
		int getMaxNo(){			return _maxNo;		}

		/// Ищем следующую после итема № after итемку. Нужно сделать экономичный поиск.
		T* getNextItem(int after){
			T* item = NULL;
			if ( after<=0 || after<_minNo || after>_maxNo || _list->isEmpty())
				return item;
			detachIter();
			// Чета влом мне в час ночи ломать голову над оптимизацией...
			typename QLinkedList<T*>::iterator it = _list->begin();
			while (it != _list->end() ) {
				item = *it;
				if (item->number()>after)
					return item;
				it++;
			}
			return NULL;
		}

		/// Очистка вектора
		void clear() {
			detachIter();
			T* item = NULL;
			typename QLinkedList<T*>::iterator it = _list->begin();
			while (it != _list->end() ) {
				item = *it;
				onDeleteItem(item);
				it = _list->erase(it);
				item->~T();
				delete item;
				item = NULL;
			}
			_maxNo = _minNo = 0;
		}

		/// Функция вызывается после создания нового итема. Возможно пригодится для ундо/редо.
		virtual void onDeleteItem(T* delItem) = 0;

		/// Функция вызывается перед удалением итема.
		virtual void onCreateItem(T* crItem) = 0;

		/// Доступ к итему.
		T* getItem(int nom, bool needCreale = false){
			T* item = NULL;
			if (findItem(nom, needCreale)){
				item = *_itSave;
				return item;
			}
			return NULL;
		}



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

		/// Просто отдаем список итемов
		QList<T*> getItemList(int statrWith = 0, int endWith = 0)
		{
			QList<T*> list;
			typename QLinkedList<T*>::iterator it = _list->begin();
			T* item = NULL;
			while (it != _list->end() ) {
				item = *it;
				list.append(item);
				it++;
			}
			return list;
		}

		/// Функция создания нового итема
		virtual T* createItem(int nom) {
			T* item = new T(nom);
			if (item) {
				onCreateItem(item);
			}

			return item;
		}


		/// Получение размера списка
		int getCountItem() {
			return _list->size();
		}
		/// проверить, существует ли итемка.
		bool itemExist(int nom) {
			return findItem(nom, false);
		}

		/// проверить, существует ли итемка.
		int getNextItemNom(int nom) {
			int retVal = -1, curVal;
			if (_list->isEmpty())
				return retVal;

			typename QLinkedList<T*>::iterator it = _list->begin();
			T* item = *it;
			while (it != _list->end() ) {
				item = *it;
				curVal = item->number();
				if (curVal > nom) {
					retVal = curVal;
					break;
				}
				it++;
			}
			return retVal;
		}


	protected:

		/// Определить Min и Max № после вставки итема в список.
		void defineMinMax(int nom) {
			if (_minNo == 0 && _maxNo == 0) {
				_minNo = _maxNo = nom;
			} else {
				if (_minNo>nom)	_minNo = nom;
				if (_maxNo<nom)	_maxNo = nom;
			}
		}

		/// Отключаемся от итератора.
		inline void detachIter() {
			_itSave	= NULL;
			_itSaveOnUse = false;
		}


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
				if (_itSave == _list->begin()){
					item = *_itSave;
					lastItemNo = item->number();
					if (lastItemNo == nom)
						return true;
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
			bool resVal = false;
			int itemNo = psItem->number();
			if ( itemNo < _minNo && _minNo>0) {
				_list->prepend(psItem);
				resVal = true;
			} else if (itemNo > _maxNo && _maxNo > 0) {
				_list->append(psItem);
				resVal = true;
			} else {
				int itemNo2 = psItem->number();
				T* item = NULL;
				typename QLinkedList<T*>::iterator it = _list->begin();
				while (it != _list->end() ) {
					item = *it;
					itemNo2 = item->number();
					if (itemNo2 > itemNo) {
						it = _list->insert(it, psItem);
						resVal = true;
						break;
					}
					it++;
				}
			}
			defineMinMax(itemNo);
			if (resVal) {
				/// иначе была фигня с поиском....
				detachIter();
				return resVal;
			}
			return false;
		}
};


} //namespace uoReport


#endif // UONUMVECTOR_H_INCLUDED
