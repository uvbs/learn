/*************************************************************************
    > File Name: min-heap.h
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon 05 Jan 2015 06:22:48 PM CST
 ************************************************************************/

#ifndef _MIN_HEAP_H__
#define _MIN_HEAP_H__

#include <stdio.h>
#include <stdlib.h>

class TimerHandler;

namespace yymobilesdk
{
	namespace heap
	{	
		class CMiniHeapElem
		{
		public:
			CMiniHeapElem() {
				m_min_heap_idx = -1;
			}

			virtual ~CMiniHeapElem() {
				m_min_heap_idx = -1;
			}
			unsigned m_min_heap_idx;
		};

		template <typename T, typename Compare>
		class CMiniHeap
		{
		public:
			typedef void (T::*classfunc)(void *);
		public:
			CMiniHeap();
			~CMiniHeap();

			int reserve(unsigned size);
			int size();
			int empty();
			T * top();
			T * pop();
			int push(T * elem);
			int erase(T * elem);
			int adjust(T * elem);
			void doall(classfunc f, void * arg);

		private:
			void ctor();
			void dtor();
			void shift_up(unsigned hole_index, T * elem);
			void shift_down(unsigned hole_index, T * elem);

		private:
			T ** mPtr;
			unsigned mCapacity;
			unsigned mSize;
			Compare mCompare;
		};

		template <typename T, typename Compare>
		CMiniHeap<T, Compare>::CMiniHeap()
		{
			ctor();
		}

		template <typename T, typename Compare>
		CMiniHeap<T, Compare>::~CMiniHeap()
		{
			dtor();
		}

		template <typename T, typename Compare>
		void CMiniHeap<T, Compare>::ctor()
		{
			mPtr		= NULL;
			mCapacity	= 0;
			mSize		= 0;
		}

		template <typename T, typename Compare>
		void CMiniHeap<T, Compare>::dtor()
		{
			if (mPtr != NULL) {
				free(mPtr);
				mPtr	= NULL;
			}
			
			mCapacity	= 0;
			mSize		= 0;
		}

		template <typename T, typename Compare>
		T * CMiniHeap<T, Compare>::top()
		{
			return mSize ? *(mPtr) : NULL;
		}

		template <typename T, typename Compare>
		T * CMiniHeap<T, Compare>::pop()
		{
			if (mSize > 0) {
				T * elem = * mPtr;
				shift_down(0u, mPtr[--mSize]);
				elem->m_min_heap_idx = -1;
				return elem;
			}
			return NULL;
		}

		template <typename T, typename Compare>
		int CMiniHeap<T, Compare>::push(T * elem)
		{
			if (reserve(mSize + 1)) {
				return -1;
			}
			shift_up(mSize ++, elem);
			return 0;
		}

		template <typename T, typename Compare>
		int CMiniHeap<T, Compare>::adjust(T * elem)
		{
			if (elem->m_min_heap_idx != -1) {
				unsigned parent = (elem->m_min_heap_idx - 1) >> 1;
				if (elem->m_min_heap_idx > 0 && mCompare(mPtr[parent], elem)) {
					shift_up(elem->m_min_heap_idx, elem);
				} else {
					shift_down(elem->m_min_heap_idx, elem);
				}
			}
			return 0;
		}

		template <typename T, typename Compare>
		int CMiniHeap<T, Compare>::erase(T * elem)
		{
			if (elem->m_min_heap_idx != -1) {
				T * last = mPtr[-- mSize];

				unsigned parent = (elem->m_min_heap_idx - 1) >> 1;
				if (elem->m_min_heap_idx > 0 && mCompare(mPtr[parent], elem)) {
					shift_up(elem->m_min_heap_idx, last);
				} else {
					shift_down(elem->m_min_heap_idx, last);
				}
				elem->m_min_heap_idx = -1;
				return 0;
			}
			return -1;
		}

		template <typename T, typename Compare>
		int CMiniHeap<T, Compare>::reserve(unsigned size)
		{
			if (mCapacity < size) {
				T ** ptr = NULL;
				unsigned capacity = mCapacity ? mCapacity * 2 : 8;
				if (capacity < size) {
					capacity = size;
				}

				ptr = (T **)realloc(mPtr, capacity * sizeof(ptr));
				if (ptr == NULL) {
					return -1;
				}

				mPtr = ptr;
				mCapacity = capacity;
			}
			return 0;
		}

		template <typename T, typename Compare>
		int CMiniHeap<T, Compare>::size()
		{
			return mSize;
		}

		template <typename T, typename Compare>
		int CMiniHeap<T, Compare>::empty()
		{
			return 0u == mSize;
		}

		template <typename T, typename Compare>
		void CMiniHeap<T, Compare>::shift_up(unsigned hole_index, T * elem)
		{
			unsigned parent = (hole_index - 1) >> 1;

			while (hole_index && mCompare(mPtr[parent], elem)) {
				mPtr[hole_index] = mPtr[parent];
				hole_index = parent;
				parent = (hole_index - 1) >> 1;
			}

			(mPtr[hole_index] = elem)->m_min_heap_idx = hole_index;
		}

		template <typename T, typename Compare>
		void CMiniHeap<T, Compare>::shift_down(unsigned hole_index, T * elem)
		{
			unsigned min_child = (hole_index + 1) << 1;

			while (min_child <= mSize) {
				min_child -= (min_child == mSize || mCompare(mPtr[min_child], mPtr[min_child - 1]));
				if (!mCompare(elem, mPtr[min_child])) {
					break;
				}

				mPtr[hole_index] = mPtr[min_child];
				hole_index = min_child;
				min_child = (hole_index + 1) << 1;
			}
			(mPtr[hole_index] = elem)->m_min_heap_idx = hole_index;
		}
		template <typename T, typename Compare>
		void CMiniHeap<T, Compare>::doall(classfunc f, void * arg)
		{
			T ** prev = mPtr;
			unsigned size = mSize;

			if (mPtr == NULL) {
				return;
			}

			for (; size -- > 0; ++ prev) {
				((* prev)->*f)(arg);
			}
		}
	}
}
#endif