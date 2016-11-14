/**************************************
*
*	© trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#ifndef UOCACHEITEMIZER_H
#define UOCACHEITEMIZER_H

#include <QLinkedList>
#include "uoReport.h"

namespace uoReport {
///\class uoCacheItemizer Шаблонный расширяемый класс для хранения структур
///\brief Шаблонный расширяемый класс для хранения структур
///
///		Грубо говоря простое хранилище структур.
template <typename T> class uoCacheItemizer
{
 	protected:
		QLinkedList<T*>* _list;
	public:
		uoCacheItemizer(){
			 _list = new QLinkedList<T*>;
		}
		virtual ~uoCacheItemizer(){
			clear();
		}
		void clear(){
			if (_list->isEmpty())
				return;
			T* item = NULL;
			typename QLinkedList<T*>::iterator it = _list->begin();
			while (it != _list->end() ) {
				item = *it;
				it = _list->erase(it);
				delete item;
				item = NULL;
			}
		}
		T* getItem(){
			T* item = NULL;
			if (!_list->isEmpty()) {
				item = _list->takeFirst();
			} else {
				item = new T;
			}

			return item;
		}

		void saveItem(T* item){
			if (item){
				_list->append(item);
			}
		}

};
}
#endif // UOCACHEITEMIZER_H
