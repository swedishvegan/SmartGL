#ifndef SORTER_IMPLEMENTATIONS_HPP
#define SORTER_IMPLEMENTATIONS_HPP

#include <time.h>
#include "./Sorter.hpp"

#define _MakeSorterDerivedClass(DerivedClassName, HelperFunctions) \
template <typename T> \
class DerivedClassName : public Sorter<T> { \
public: \
	\
	template <typename Subscriptable> \
	DerivedClassName(Subscriptable& s, unsigned long size) : Sorter<T>(s, size) { \
		\
		clock_t start = clock(); \
		sort(); \
		clock_t end = clock(); \
		Sorter<T>::time = (double)(end - start) / (double)CLOCKS_PER_SEC; \
		\
	} \
	\
protected: \
	\
	void sort(); \
	\
	HelperFunctions \
	\
};

_MakeSorterDerivedClass(InsertionSorter, )
_MakeSorterDerivedClass(SelectionSorter, )
_MakeSorterDerivedClass(MergeSorter, void merge(T*, unsigned long, unsigned long, unsigned long); void sort_recursive(T*, unsigned long, unsigned long);)
_MakeSorterDerivedClass(HeapSorter, void heapify(unsigned long, unsigned long);)
_MakeSorterDerivedClass(QuickSorter, void partition(unsigned long, unsigned long);)

#define _SorterSwap(i, j) \
T temp = data[i]; \
data[i] = data[j]; \
data[j] = temp;

template <typename T>
void InsertionSorter<T>::sort() {

	T* data = Sorter<T>::data;
	bool done = false;
	unsigned long numFound = 0u;

	while (!done) {

		done = true;
		for (unsigned long i = numFound; i < Sorter<T>::siz - 1u; i++) {

			if (data[i + 1u] < data[i]) {

				unsigned long j = i;
				while (data[j + 1u] < data[j]) {

					// Move the out-of-place element backwards in the list to where it belongs.

					_SorterSwap(j, j + 1u)
					Sorter<T>::numComparisons++; // Comparison was made.
					if (j == 0u) break;
					j--;

				}

				done = false;
				numFound++;

			}

			Sorter<T>::numComparisons++; // Comparison was made.

		}
	}
}

template <typename T>
void SelectionSorter<T>::sort() {

	T* data = Sorter<T>::data;

	for (unsigned long i = 0u; i < Sorter<T>::siz - 1u; i++) {

		unsigned long minIndex = i;
		T minValue = data[minIndex];

		for (unsigned long j = i + 1u; j < Sorter<T>::siz; j++) {

			// Find minimum value and get its index.

			if (data[j] < minValue) {

				minValue = data[j];
				minIndex = j;

			}

			Sorter<T>::numComparisons++; // Comparison was made.

		}

		// Insert minimum value in the corresponding place in the list.

		_SorterSwap(minIndex, i)

	}

}

template <typename T>
void MergeSorter<T>::merge(T* tempData, unsigned long leftIndex, unsigned long middleIndex, unsigned long rightIndex) {

	// LH list covers [leftIndex, middleIndex) and RH list covers [middleIndex, rightIndex).

	if (leftIndex == middleIndex || middleIndex == rightIndex) return;
	
	T* data = Sorter<T>::data;

	// Copy relevant part of data into tempData.
	for (unsigned long i = leftIndex; i < rightIndex; i++) tempData[i] = data[i];

	// Merge LH and RH lists in tempData into data.

	unsigned long LH_idx = leftIndex;
	unsigned long RH_idx = middleIndex;
	
	for (unsigned long i = leftIndex; i < rightIndex; i++) {

		// Determine if the LH or RH value is smaller.

		bool LH_smaller;
		if (RH_idx == rightIndex) LH_smaller = true; // This makes sure tempData[LH_idx] isn't compared to garbage on the next line.
		else LH_smaller = LH_idx < middleIndex && tempData[LH_idx] < tempData[RH_idx];

		unsigned long& min_idx = LH_smaller ? LH_idx : RH_idx;
		data[i] = tempData[min_idx];
		min_idx++;
		Sorter<T>::numComparisons++; // Comparison was made.

	}

}

template <typename T>
void MergeSorter<T>::sort_recursive(T* tempData, unsigned long leftIndex, unsigned long rightIndex) {

	// The part of the array addressed by this function is [leftIndex, rightIndex)

	if (leftIndex > rightIndex || rightIndex - leftIndex < 2u) return;

	unsigned long middleIndex = (leftIndex + rightIndex) / 2u;
	sort_recursive(tempData, leftIndex, middleIndex);
	sort_recursive(tempData, middleIndex, rightIndex);
	merge(tempData, leftIndex, middleIndex, rightIndex);

}

template <typename T>
void MergeSorter<T>::sort() {

	T* tempData = new T[Sorter<T>::siz];
	sort_recursive(tempData, 0u, Sorter<T>::siz);
	delete[] tempData;

}

template <typename T>
void HeapSorter<T>::heapify(unsigned long heapSize, unsigned long nodeIndex) {

	// Helper function for heap sort.

	if (nodeIndex >= heapSize) return;

	unsigned long leftNodeIndex = nodeIndex * 2u + 1u;
	unsigned long rightNodeIndex = nodeIndex * 2u + 2u;
	unsigned long largestChildIndex = nodeIndex;
	T* data = Sorter<T>::data;

	if (leftNodeIndex < heapSize && data[leftNodeIndex] > data[largestChildIndex]) largestChildIndex = leftNodeIndex;
	if (rightNodeIndex < heapSize && data[rightNodeIndex] > data[largestChildIndex]) largestChildIndex = rightNodeIndex;

	Sorter<T>::numComparisons += 2u; // Two comparisons were made.

	if (largestChildIndex > nodeIndex) {

		_SorterSwap(largestChildIndex, nodeIndex)
		heapify(heapSize, largestChildIndex);

	}

}

template <typename T>
void HeapSorter<T>::sort() {

	T* data = Sorter<T>::data;
	unsigned long heapSize = Sorter<T>::siz;

	// Build max heap tree.

	for (unsigned long i = heapSize - 1u; i > 0u; i--) {

		heapify(heapSize, i);

	}
	heapify(heapSize, 0u); // Didn't want the loop to go to zero because i is unsigned.

	// The meat of the heap sort algorithm

	while (heapSize > 1u) {

		// Swap root element with last element in array.
		_SorterSwap(0u, heapSize - 1u)

		// Decrease heap size by one and call heapify on the root node.

		heapSize -= 1u;
		heapify(heapSize, 0u);

	}

}

template <typename T>
void QuickSorter<T>::partition(unsigned long startIndex, unsigned long endIndex) {

	// Works on portion [startIndex, endIndex) of array.
	
	if (endIndex > Sorter<T>::siz || startIndex > endIndex || endIndex - startIndex < 2u) return;
	
	unsigned long leftPartition_end = startIndex;
	T* data = Sorter<T>::data;
	T pivot = data[endIndex - 1u];

	// If an element is less than pivot, move it over to the left.

	for (unsigned long i = startIndex; i < endIndex - 1u; i++) {

		if (data[i] < pivot) {

			_SorterSwap(i, leftPartition_end)
			leftPartition_end++;

		}

		Sorter<T>::numComparisons++; // Comparison was made.

	}
	
	// Put the pivot between the two partitions
	_SorterSwap(leftPartition_end, endIndex - 1u)
	
	// Call partition on each smaller half of the array
	partition(startIndex, leftPartition_end);
	partition(leftPartition_end + 1u, endIndex);

}

template <typename T>
void QuickSorter<T>::sort() { partition(0u, Sorter<T>::siz); }

#endif