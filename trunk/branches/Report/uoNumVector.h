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
		bool _itSaveOnUse;
		rptSize _def_size;

	public:
		uoNumVector()
			: _itSaveOnUse(false)
		{
			_list = new QLinkedList<T*>;
			_def_size = -1;
		};
		~uoNumVector()
		{
			_itSave = NULL;
			if (_list->size() == 0)
				return;
			clear();
		}

		/// Очистка вектора
		void clear() {
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
			while (it != _list->end() ) {
				item = *it;
				if (item->number() > lastNo)
					item->setNumber(item->number() - cnt);
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


		/// Поиск итема по номеру в списке. При необходимости происходит создание итема. \n Просто позиционируемся на нужном итераторе
		bool findItem(int nom, bool needCreate = false)
		{
			T* item = NULL;
			if (_list->isEmpty() && !needCreate)
				return false;

			if (_list->isEmpty() && needCreate) {
				item = createItem(nom);
				_itSaveOnUse = false;
				_list->append(item);
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

			uoSearchDirection srchDirection = toUp;
			int lastInemNom = 0;

			item = *_itSave;
			if (item->number() == nom)
				return true;
			else if (item->number()>nom) {
				while (_itSave != _list->begin()){
					item = *_itSave;
					lastInemNom = item->number();
					if (lastInemNom == nom)
						return true;
					else if (lastInemNom < nom)
						break;
					_itSave--;
				}
				_itSaveOnUse = false;
			} else if (item->number()<nom) {
				srchDirection = toDown;
				_itSaveOnUse = false;
				while (_itSave != _list->end() ){
					item = *_itSave;
					lastInemNom = item->number();
					if (lastInemNom == nom)
						return true;
					else if (lastInemNom > nom) {
						_itSaveOnUse = true;
						break;
					}
					_itSave++;
				}
			}

			if (needCreate) {
				item = createItem(nom);

				if (srchDirection == toDown) {
					if (!_itSaveOnUse){
						_list->append(item);
						_itSave = _list->end();
					}	else if (lastInemNom > nom){
						_itSave = _list->insert(_itSave, item);
					}
				} else if (srchDirection == toUp) {
					if (!_itSaveOnUse){
						_list->prepend(item);
						_itSave = _list->begin();
					}	else if (lastInemNom < nom){
						_itSave++;
						if (_itSave == _list->end()) {
							_list->append(item);
							_itSave = _list->end();
						} else {
							_itSave = _list->insert(_itSave, item);
						}
					}
				}
				_itSaveOnUse = false;
				goto reserch;
			}
			return false;
		}
	public:
 		/// Получить размер итема, а если итем не существует, тогда его дефолтный размер..
		rptSize getSize(int nom, bool isDef = true){
			if (findItem(nom)) {
				T* item = *_itSave;
				if (item)
					return item->size(isDef);
			}
			return _def_size;
		}

		/// Установить размер итема, а если итем не существует, создать и установить
		bool	setSize(int nom, rptSize size, bool isDef = true){
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
		void setDefSize(rptSize size) {
			_def_size = size;
		}

};


} //namespace uoReport


#endif // UONUMVECTOR_H_INCLUDED
