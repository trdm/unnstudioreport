#ifndef UONUMVECTOR_H_INCLUDED
#define UONUMVECTOR_H_INCLUDED

/**************************************
*
*	(c) trdm, as Troshin D. V. 2008
*  	GNU GPL3
*  	unNStudio project
*
***************************************/

#include <QVector>
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

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!	стоит заметить, что нумерация начинается с "0",!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 	protected:
		QVector<T*>* m_map;
		typename QVector<T*>::iterator _itSave;
		int m_maxNo, m_minNo;
		int m_iterNo; // для обхода getFirst/getNext
		T* m_foundItem;

	public:
		uoNumVector()
		{
			m_map = new QVector<T*>;
			m_maxNo = m_minNo = 0;
			m_iterNo = -1;
			m_foundItem = 0;
		};
		virtual ~uoNumVector()	{
			_itSave = NULL;
			if (m_map->size() > 0){
				clear();
			}

			delete m_map;
		}

inline	int getMinNo(){			return m_minNo;		}
inline	int getMaxNo(){			return m_maxNo;		}

		/// Ищем следующую после итема № after итемку. Нужно сделать экономичный поиск.
		T* getNextItem(int after){
			T* item = NULL;
			int index = after - 1;

			if ( after<=0 || after<m_minNo || after>m_maxNo || m_map->isEmpty())
				return item;
			detachIter();
			// Чета влом мне в час ночи ломать голову над оптимизацией...
			for(int i = index; i<m_map->size(); i++) {
				item = m_map->at(i);
				if (item){
					if (item->number()>after)
						return item;
				}
			}
			return NULL;
		}
		T* getFirst(){
			T* item = NULL;
			if (m_map->isEmpty())
				return NULL;
			detachIter();
			for(m_iterNo = 0; m_iterNo<m_map->size(); m_iterNo++){
				item = m_map->at(m_iterNo);
				if (item)
					return item;
			}
			return NULL;
		}
		T* getNext(){
			T* item = NULL;
			if (m_iterNo == -1 || m_map->size() == 0)
				return NULL;
			m_iterNo += 1;
			for(; m_iterNo<m_map->size(); m_iterNo++){
				item = m_map->at(m_iterNo);
				if (item)
					return item;
			}
			return NULL;
		}

		/// Очистка вектора
		void clear() {
			detachIter();
			T* item = NULL;
			typename QVector<T*>::iterator it = m_map->begin();
			while (it != m_map->end() ) {
				item = *it;
				if (item)
					onDeleteItem(item);
				it = m_map->erase(it);
				delete item;
				item = NULL;
			}
			m_maxNo = m_minNo = 0;
		}

		/// Функция вызывается после создания нового итема. Возможно пригодится для ундо/редо.
		virtual void onDeleteItem(T* delItem) // = 0;
        {   Q_UNUSED(delItem); }

		/// Функция вызывается перед удалением итема.
		virtual void onCreateItem(T* crItem) // = 0;
        { Q_UNUSED(crItem); }

		inline bool isEmpty() const
		{
			if (m_map->isEmpty())
				return true;
			return false;
		}

		/// Доступ к итему.
		T* getItem(const int& nom, bool needCreale = false){
			T* item = NULL;
			int index = nom - 1;
			if (index < 0)
				return 0;
			if (findItem(nom, needCreale)){
				item = m_map->at(index);
				return item;
			}
			return NULL;
		}

		/// Используется при вставке значений, незачем добавлять итемы, нужно просто сдвинуть номера..
		void addEmptyItems(const int& nom, int cnt = 1)
		{
			if (nom<0 || cnt<=0)
				return;
			if (m_map->count() == 0){
				m_map->insert(0, nom + cnt - 1, 0);
				return;
			}
			detachIter();
			int pos = nom-1;
			if (m_map->count()>=pos ){
				m_map->insert(pos, cnt , 0);
			} else {
				return; // иначе вылет..
			}

			T* item = NULL;
			for (int i = nom + cnt - 1; i<m_map->size(); i++){
				item = m_map->at(i);
				if (item){
					m_maxNo = item->number();
					if (m_maxNo >= nom) {
						m_maxNo = m_maxNo + cnt;
						item->setNumber(m_maxNo);
					}
				}
			}
		}



		/// удаление итема/итемов
		void deleteItem(const int& nom, int cnt = 1)
		{
			if (cnt<=0 || nom<=0)
				return;
			T* item = NULL;

			if (nom>m_map->size())
				return;

			int index = nom - 1;
			int counter = cnt;

			for (int i = index; i<m_map->size(); i++) {
				item = m_map->at(i);
				if (item){
					onDeleteItem(item);
					m_map->replace(i, 0);
				}
				--counter;
				if (counter == 0)
					break;
			}
			counter = qMin(m_map->size() - index, cnt);
			m_map->remove(index, counter);

			detachIter();

			/*
				Нужно сдвинуть номера итемов вверх на количество удаленных,
				начиная с последнего удаленного итема или с того номера,
				который должен быть удален.
			*/
			if (m_map->count() == 0)
			{
				m_minNo = m_maxNo = -1;
				return;
			}

			typename QVector<T*>::iterator it = m_map->begin();
			while (it != m_map->end() ) {
				item = *it;
				if (item) {
					m_maxNo = item->number();
					if (m_maxNo > index) {
						m_maxNo = m_maxNo - cnt;
						item->setNumber(m_maxNo);
					}
				}
				it++;
			}
		}


		/// Функция создания нового итема
		virtual T* createItem(const int& nom) {
			T* item = new T(nom);
			if (item) {
				onCreateItem(item);
			}

			return item;
		}


		/// Получение размера списка
		int getCountItem() {
			// В случае вектора лучше наверное подсчитать действительные члены?
			T* item = 0;
			int cntr = 0;
			for(int i = 0; i<m_map->size(); i++){
				item = m_map->at(i);
				if (item)
					cntr += 1;
			}
			return cntr; //m_map->size();
		}
		/// проверить, существует ли итемка.
		bool itemExist(const int& nom) {
			return findItem(nom, false);
		}

		/// проверить, существует ли итемка.
		int getNextItemNom(const int& nom) {
			int retVal = -1, curVal;
			if (m_map->isEmpty())
				return retVal;

			typename QVector<T*>::iterator it = m_map->begin();
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
		void defineMinMax(const int& nom) {
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
			m_iterNo = -1;
		}


		/// Поиск итема по номеру в списке. При необходимости происходит создание итема. \n Просто позиционируемся на нужном итераторе
		bool findItem(const int& nom, bool needCreate = false)
		{
			T* item = NULL;

			int index = nom-1;
			if (index<0){
				index = 0;
				///\todo - надо бы выяснить, откуда этот подарочек "приходит", а он приходит....
				//Q_ASSERT(false);
				return false;
			}


			if (m_map->count() < nom){
				if (!needCreate) {
					return false;
				}
				else {
					m_map->resize(nom);
				}
			}
			item = m_map->at(index);
			// Сюда доходим если не найшли итем.
			if (needCreate && !item) {
				item = createItem(nom);
				if (addItem(item)) {
					return true;
				} else {
					return false;
				}
			}
			return true;
		}

		/// Добавим итем в список, для надежности прийдется пробежаться по нему
		bool addItem(T* psItem){
			bool resVal = true;
			int itemNo = psItem->number();
			int index = itemNo-1;

			if (m_map->count() < itemNo)
				m_map->resize(itemNo);

			m_map->replace(index, psItem);
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
