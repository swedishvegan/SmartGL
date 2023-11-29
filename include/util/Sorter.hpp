#ifndef SORTER_HPP
#define SORTER_HPP

template <typename T>
class Sorter {
public:

	// Derived classes of Sorter are automatically sorted upon construction.
	template <typename Subscriptable>
	Sorter(Subscriptable& s, unsigned long size);

	virtual ~Sorter();

	// Makes sure the list is sorted.
	bool verify();

	// Included this operator for testing purposes
	T& operator [] (unsigned long index);

	unsigned long size();

	double getElapsedTime();

	unsigned long getNumComparisons();

protected:

	T* data;
	unsigned long siz;
	double time;
	unsigned long numComparisons;

	virtual void sort() = 0;

};

template <typename T>
template <typename Subscriptable>
Sorter<T>::Sorter(Subscriptable& s, unsigned long size) : data(nullptr), siz(size), time(0.0), numComparisons(0u) {
	
	if (siz == 0u) siz++;	// Don't feel like bothering with exception handling here.
	data = new T[siz];
	for (unsigned long i = 0u; i < size; i++) data[i] = s[i];

}

template <typename T>
Sorter<T>::~Sorter() { if (data) delete[] data; }

template <typename T>
bool Sorter<T>::verify() {

	for (unsigned long i = 0u; i < siz - 1u; i++) if (data[i] > data[i + 1u]) return false;
	return true;

}

template <typename T>
T& Sorter<T>::operator [] (unsigned long index) { return data[index % siz]; }

template <typename T>
unsigned long Sorter<T>::size() { return siz; }

template <typename T>
double Sorter<T>::getElapsedTime() { return time; }

template <typename T>
unsigned long Sorter<T>::getNumComparisons() { return numComparisons; }

#endif