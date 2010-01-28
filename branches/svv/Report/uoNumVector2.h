#ifndef UONUMVECTOR2_H_INCLUDED
#define UONUMVECTOR2_H_INCLUDED

/**************************************
*
*	© trdm, as Troshin D. V. 2009
*  	GNU GPL3
*  	unNStudio project
*
***************************************/
#include <QMap>
#include "uoReport.h"

namespace uoReport {

///\class uoNumVector2 Шаблонный расширяемый класс для хранение и обработки длинных нумерованных структур.
///\brief Шаблонный расширяемый класс для хранение и обработки длинных нумерованных структур.
///
/// Определены стандартные операции установки размера, запроса размера, чистка списка, \n
/// удаление диапазонов итемов. Все остальное нужно определить в производных классах.
template <typename T>
class uoNumVector2
{
 	protected:
		QMap<int, T*>* m_map;
		typename QMap<int, T*>::iterator _itSave;
		bool m_itSaveOnUse; ///< итератор _itSave - используется.
		int m_maxNo, m_minNo;

	public:
		uoNumVector2()
			: m_itSaveOnUse(false)	{
			m_map = new QMap<int, T*>;
			m_maxNo = m_minNo = 0;
		};
		virtual ~uoNumVector2()	{
			_itSave = NULL;
			if (m_map->size() == 0){
				delete m_map;
				return;
			}
			clear();
			delete m_map;
		}

		int 	getMinNo(){	return m_minNo;		}
		int 	getMaxNo(){	return m_maxNo;		}
		bool 	isEmpty() {	return m_map->isEmpty(); }

		/// Ищем следующую после итема № after итемку. Нужно сделать экономичный поиск.
		T* getNextItem(int after){
			T* item = NULL;
			if ( after<=0 || after<m_minNo || after>m_maxNo || m_map->isEmpty())
				return item;
			// Чета влом мне в час ночи ломать голову над оптимизацией...
			typename QMap<int, T*>::iterator it = m_map->upperBound(after);
			while (it != m_map->end() ) {
				item = *it;
				if (item->number()>after)
					return item;
				it++;
			}
			return NULL;
		}
		T* getFirst(){
			T* item = NULL;
			if (m_map->isEmpty ())
				return NULL;

			// Чета влом мне в час ночи ломать голову над оптимизацией...
			_itSave = m_map->upperBound(0);
			if (_itSave != m_map->end())
			{
				item = *_itSave;
				return item;
			}
			return NULL;
		}
		T* getNext(){
			T* item = NULL;
			if (m_itSaveOnUse)
				return NULL;
			_itSave++;
			if (_itSave != m_map->end())
			{
				item = *_itSave;
				return item;
			}
			return NULL;
		}

		/// Очистка вектора
		void clear() {
			T* item = NULL;
			typename QMap<int, T*>::iterator it = m_map->begin();
			while (it != m_map->end() ) {
				item = *it;
				onDeleteItem(item);
				it = m_map->erase(it);
				item->~T();
				delete item;
				item = NULL;
			}
			m_maxNo = m_minNo = 0;
		}

		/// Функция вызывается после создания нового итема. Возможно пригодится для ундо/редо.
		virtual void onDeleteItem(T* delItem) = 0;

		/// Функция вызывается перед удалением итема.
		virtual void onCreateItem(T* crItem) = 0;

		/// Доступ к итему.
		T* getItem(int nom, bool needCreate = false){
			T* item = NULL;
			if (m_map->contains(nom)){
				item = m_map->value(nom);
			} else if (needCreate){
				item = createItem(nom);
				addItem(item);
			}
			return item;
		}

		/// Используется при вставке значений, незачем добавлять итемы, нужно просто сдвинуть номера..
		void addEmptyItems(int nom, int cnt = 1)
		{
			if (m_map->count() == 0 || nom<=0 || cnt<=0)
				return;

			T* item = NULL;
			int nomItem = 0;
			QLinkedList<T*> list;
			typename QMap<int, T*>::iterator it = m_map->lowerBound(nom);
			typename QMap<int, T*>::iterator itprev;
			while (it != m_map->end()) {
				itprev = it;
				item = it.value();
				m_maxNo = item->number();
				++it;
				nomItem = itprev.key();
				if (nomItem >= nom){
					m_map->erase(itprev);
					m_maxNo = m_maxNo + cnt;
					item->setNumber(m_maxNo);
					list.append(item);
				}
			}
			if (list.count()>0){
				typename QLinkedList<T*>::iterator it2 = list.begin();
				while (it2 != list.end() ) {
					item = *it2;
					m_map->insert(item->number(),item);
					++it2;
				}
			}
		}



		/// удаление итема/итемов
		void deleteItem(int nom, int cnt = 1)
		{
			if (cnt<=0)
				return;
			T* item = NULL;
			int lastNo = nom + cnt - 1;
			int nomItem = 0;

			QLinkedList<T*> list;
			typename QMap<int, T*>::iterator it = m_map->lowerBound(nom);
			typename QMap<int, T*>::iterator itprev;
			while (it != m_map->end()) {
				itprev = it;
				item = it.value();
				m_maxNo = item->number();
				++it;
				nomItem = itprev.key();
				if (nomItem <= lastNo){
					m_map->erase(itprev);
					m_maxNo = m_maxNo - cnt;
					onDeleteItem(item);
					item->~T();
					delete item;
				} else if (nomItem>lastNo){
					m_map->erase(itprev);
					m_maxNo = m_maxNo - cnt;
					item->setNumber(m_maxNo);
					list.append(item);

				}
			}
			if (list.count()>0){
				typename QLinkedList<T*>::iterator it2 = list.begin();
				while (it2 != list.end() ) {
					item = *it2;
					m_map->insert(item->number(),item);
					++it2;
				}
			}
		}

		/// Просто отдаем список итемов
		QList<T*> getItemList(int statrWith = 0, int endWith = 0)
		{
			QList<T*> list;
			typename QMap<int, T*>::iterator it = m_map->begin();
			T* item = NULL;
			while (it != m_map->end() ) {
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
			return m_map->size();
		}
		/// проверить, существует ли итемка.
		bool itemExist(int nom) {
			return findItem(nom, false);
		}

		/// проверить, существует ли итемка.
		int getNextItemNom(int nom) {
			int retVal = -1, curVal;
			if (m_map->isEmpty())
				return retVal;

			typename QMap<int, T*>::iterator it = m_map->begin();
			T* item = *it;
			while (it != m_map->end() ) {
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
			if (m_minNo == 0 && m_maxNo == 0) {
				m_minNo = m_maxNo = nom;
			} else {
				if (m_minNo>nom)	m_minNo = nom;
				if (m_maxNo<nom)	m_maxNo = nom;
			}
		}

		/// Отключаемся от итератора.
		inline void detachIter() {
			_itSave	= NULL;
			m_itSaveOnUse = false;
		}


		/// Поиск итема по номеру в списке. При необходимости происходит создание итема. \n Просто позиционируемся на нужном итераторе
		bool findItem(int nom, bool needCreate = false)
		{
			bool retVal = false;
			T* item = NULL;
			if (!m_map->contains(nom)){
				if (needCreate) {
					item = createItem(nom);
					if (item){
						addItem(item);
						_itSave = m_map->find(nom);
						if (_itSave != m_map->end())
							retVal = true;
					}
				}
			} else {
				_itSave = m_map->find(nom);
				if (_itSave != m_map->end())
					retVal = true;
			}

			return retVal;
		}

		/// Добавим итем в список, для надежности прийдется пробежаться по нему
		bool addItem(T* psItem){
			bool resVal = false;
			int itemNo = psItem->number();
			if (!m_map->contains(itemNo)){
				m_map->insert(itemNo, psItem);
				resVal = true;
				defineMinMax(itemNo);
			}
			return resVal;
		}
};


} //namespace uoReport


#endif // UONUMVECTOR2_H_INCLUDED
