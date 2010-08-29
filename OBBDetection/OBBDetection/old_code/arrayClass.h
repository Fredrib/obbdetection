#include "arraysInc.h"

// JONATHAN TOMPSON
// DOES NOT ALLOW ARRAYS OF POINTERS (NEED TO SPECIFY PARTIAL SPECILIZATION FOR POINTERS)
// See: http://www.learncpp.com/cpp-tutorial/146-partial-template-specialization/

#ifndef ARRAYCLASS // allows multiple includes of the same file
#define ARRAYCLASS

template <typename T>
class Array {
public:
	// ******* CONSTURCTORS AND DESTRUCTORS *******
	Array(const Array &a);							// copy constructor: O(n) - uses memcpy
	Array(unsigned long inSize = 0);				// constructor: O(1) - doesn't initialize!!
	~Array(void);									// default destructor: O(1)

	// ******* SELECTORS AND MODIFIERS *******
	unsigned long getSize() {return arraySize;};	// O(1)
	void setSize(unsigned long inSize);				// O(n) - reallocs if growing or shrinking
	void clear();									// clear memory and realloc size to startSize - O(n)
	void print(unsigned long length = 0);			// Print first length elements (0 = all) - O(n) - linear search
	void add(T newElem);							// add to end and increase size O(2) - amortized time, may double array
	void setElem(T value, unsigned long index);		// Set an aray index - O(1)

	void insertionSort(bool forward = true);		// O(n^2)
	void selectionSort(bool forward = true);		// O(n^2)
//	void shellSort(bool forward = true);			// O(n^(3/2))
//  void heapSort(bool forward = true);				// O(n.log(n))
//	void quickSort(bool forward = true);			// O(n.log(n))
	void mergeSort(bool forward = true);			// O(n.log(n))
	void shuffle(void);								// O(n)

	// ******* OPERATORS *******
	Array<T>& operator = (const Array<T> &a);		// O(n) - uses memcpy
	bool operator == (const Array<T> &a);			// O(n) - linear search

	// ******* STATIC *******
	static unsigned long howManyArrays;				// keep track of created instances (looking for memory leaks)

	// ******* EXCEPTIONS *******
	enum exception { MEMFAIL, OUTOFBOUNDS }; // exception enum 

private:
	unsigned long arraySize;
	unsigned long allocatedSize; // will only growing or shrinking by a factor of 2
	T* pArray;
	const static unsigned long startSize = 32; // Must be a factor of 2
	// mergeSort helper functions
	void mergeSort(unsigned long start, unsigned long finish, T* result, bool forward);
	void merge(unsigned long start, unsigned long mid, unsigned long finish, T* result, bool forward);
};

//************************************************************************
//********** MEMBER FUNCTIONS, MUST BE IN .h FILE FOR TEMPLATES **********
//************************************************************************

// ******* CONSTURCTORS AND DESTRUCTORS *******
// ******************************
// ******** CONSTRUCTOR *********
// ******************************
template <typename T>
Array<T>::Array(unsigned long inSize /* =0 */) 
{
	if(inSize < startSize){
		pArray = (T*) new T[startSize]; // allocate minimum size array
		allocatedSize = startSize;
	} else {
		pArray = (T*) new T[pow(2,ceil(log((float)inSize)/log((float)2)))]; // round up to the nearest factor of 2
		allocatedSize = (unsigned long) pow(2,ceil(log((float)inSize)/log((float)2)));
	}
	if(pArray == NULL && inSize != 0) {throw MEMFAIL;}
	arraySize = inSize;
	howManyArrays ++;
}
// ******************************
// ****** COPY CONSTRUCTOR ******
// ******************************
template <typename T> 
Array<T>::Array(const Array &a)
{
	// pArray is not yet allocated --> a bad pointer!
    pArray = (T*) new T[a.allocatedSize];
	if (pArray == NULL) {throw MEMFAIL;}
    memcpy(pArray, a.pArray, sizeof(T)*a.allocatedSize);
	// memcpy call -- coping memory contents 
    arraySize = a.arraySize;
	allocatedSize = a.allocatedSize;
}
// ******************************
// ***** DEFAULT DESTRUCTOR *****
// ******************************
template <typename T>
Array<T>::~Array(void)
{
	if(pArray != NULL) { delete[] pArray; }
	pArray = NULL;
	howManyArrays --;
}
// ** SELECTORS AND MODIFIERS ***
// ******************************
// ********** setSize ***********
// ******************************
template <typename T>
void Array<T>::setSize(unsigned long inSize)
{
	arraySize = inSize;
	if(inSize != 0) 
	{
		// Need to realloc if new size is larger or less than half than allocated size
		if((inSize>allocatedSize) || (inSize<allocatedSize/2)) 
		{
			allocatedSize = pow(2,ceil(log((float)inSize)/log((float)2)));
			cout << "\nresizing array to " << allocatedSize << "\n\n";
			pArray = (T*) realloc (pArray, allocatedSize * sizeof(Array<T>));
			if (pArray == NULL) {throw MEMFAIL;}
		}
	} else {clear();}
}
// ******************************
// *********** clear ************
// ******************************
template <typename T>
void Array<T>::clear(void) 
{
	arraySize = 0;
	allocatedSize = startSize;
	pArray = (T*) realloc (pArray, startSize * sizeof(Array<T>));
}
// ******************************
// *********** print ************
// ******************************
template <typename T>
void Array<T>::print(unsigned long length)
{
	unsigned long printLength = 0;
	if(length > arraySize) {
		printLength = arraySize;
	} else {
		printLength = (length == 0) ? arraySize : length;
	}
	for(unsigned long i=0;i<=(printLength-2);i++) {
		cout<<i<<":"<<pArray[i]<<",  ";
	}
	cout<<printLength-1<<":"<<pArray[printLength-1]<<"\n";
}
// ******************************
// ************ add *************
// ******************************
template <typename T>
void Array<T>::add(T newElem)
{
	setSize(arraySize + 1);
	pArray[arraySize-1] = newElem;
}
// ******************************
// *********** setElem **********
// ******************************
template <typename T>
void Array<T>::setElem(T value, unsigned long index )
{
	if(index > (arraySize-1)) {
		cout << "\nArray index " << index << " is out of bounds! Throwing exception.\n\n";
		throw OUTOFBOUNDS; }
	pArray[index] = value;
}
// ******************************
// ******* selectionSort ********
// ******************************
template <typename T>
void Array<T>::selectionSort( bool forward )
{
	if(arraySize<1 || pArray == NULL) {
		cout << "Array is empty, nothing to sort! Throwing exception.\n\n";
		throw OUTOFBOUNDS;
	}
	unsigned long index_of_smallest = 0;
	T value_of_smallest;
	for(unsigned long i=0; i<arraySize;i++) {  // find next smallest / largest
		index_of_smallest = i;
		for(unsigned long j=i; j<arraySize;j++) {
			if((pArray[j] < pArray[index_of_smallest] && forward) || (pArray[j] > pArray[index_of_smallest] && !forward))
				index_of_smallest = j;
		}
		// swap smallest with the current index
		if(index_of_smallest != i) {
			value_of_smallest = pArray[index_of_smallest];
			pArray[index_of_smallest] = pArray[i];
			pArray[i] = value_of_smallest;
		}
	}
}
// ******************************
// ******* insertionSort ********
// ******************************
template <typename T>
void Array<T>::insertionSort( bool forward )
{
	if(arraySize<1 || pArray == NULL) {
		cout << "Array is empty, nothing to sort! Throwing exception.\n\n";
		throw OUTOFBOUNDS;
	}
	T value_to_sort;
	for(unsigned long i=1; i<arraySize;i++) {  // Place the next value
		value_to_sort = pArray[i];
		for(unsigned long j=0; j<=i;j++) {
			if((pArray[j] > value_to_sort && forward) || (pArray[j] < value_to_sort && !forward)) {
				// push the other value forward to insert
				for(unsigned long m = i; m>j; m--) {
					pArray[m] = pArray[m-1];
				}
				pArray[j] = value_to_sort;
				break;
			}
		}
	}
}
// ******************************
// ********* mergeSort **********
// ******************************
template <typename T>
void Array<T>::mergeSort( bool forward )
{
	if(arraySize<1 || pArray == NULL) {
		cout << "Array is empty, nothing to sort! Throwing exception.\n\n";
		throw OUTOFBOUNDS;
	}
	// JUST A WRAPPER FUNCTION TO GET MERGE STARTED --> O(n) storage: could do better
	T * tempArray = (T*) new T[allocatedSize];
	mergeSort(0,arraySize-1,tempArray, forward);
	delete [] tempArray;
}
template <typename T>
void Array<T>::mergeSort(unsigned long start, unsigned long finish, T* result, bool forward)
{
	if(start < finish) {
		unsigned long mid = (start+finish)/2;
		mergeSort(start,mid,result,forward);
		mergeSort(mid+1,finish,result,forward);
		merge(start,mid,finish,result,forward);
	}	
}
template <typename T>
void Array<T>::merge(unsigned long start, unsigned long mid, unsigned long finish, T* result, bool forward)
{
	unsigned long cur_index = start;
	unsigned long left_index = start;
	unsigned long right_index = mid+1;
	while((left_index<=mid) && (right_index<=finish)) {
		if((pArray[left_index] <= pArray[right_index] && forward) || (pArray[left_index] > pArray[right_index] && !forward)) { 
			result[cur_index] = pArray[left_index]; // take from left
			left_index ++;
		} else {
			result[cur_index] = pArray[right_index]; // take from right
			right_index ++;
		}
		cur_index ++;
	}
	if(right_index<=finish) {
		while(right_index<=finish) { // insert the rest of the right side
			result[cur_index] = pArray[right_index];
			right_index ++;
			cur_index ++;
		}
	} else {
		while(left_index<=mid) { // insert the rest of the left side
			result[cur_index] = pArray[left_index];
			left_index ++;
			cur_index ++;
		}
	}
	// Update the origional array
	for(cur_index=start;cur_index<=finish;cur_index++) 
		pArray[cur_index]=result[cur_index];

}
// ******************************
// ********** shuffle ***********
// ******************************
// http://en.wikipedia.org/wiki/Fisher–Yates_shuffle - O(n) shuffle
template <typename T>
void Array<T>::shuffle(void) 
{
	// srand((unsigned)time(0)); // seed the number generator
	srand(0);
	unsigned long rand_index; T rand_index_value;
	for(unsigned long i = arraySize-1;i>0;i--) {
		rand_index = 0 + (unsigned long)((float)((i-0)*rand())/(RAND_MAX + 1.0));
		rand_index_value = pArray[rand_index];
		pArray[rand_index] = pArray[i];
		pArray[i] = rand_index_value;
	}
}
// ********* OPERATORS **********
// ******************************
// **********    =    ***********
// ******************************
template <typename T>
Array<T>& Array<T>::operator = (const Array &a)
{
	if(this == &a) {return *this;}		// if both point to the same memory
	if(a.arraySize == 0) {clear();}	// RHS is empty so clear the LHS
	SetSize(a.arraySize);
	memcpy(pArray, a.pArray, sizeof(T)*a.size);
	if (pArray == NULL) {throw MEMFAIL;}
	return *this;
}
// ******************************
// **********    ==    **********
// ******************************
template <typename T>
bool Array<T>::operator == (const Array &a)
{
	if(this == &a) {return true;}		// if both point to the same memory
	if(arraySize != a.arraySize) {return false;}
	for(unsigned long i=0; i<=(arraySize-1);i++) {
		if(pArray[i] != a.pArray[i]){ return false; }
	}
	return true;
}
// **************************************
// ******* STATIC INITIALIZATIONS *******
// **************************************
template <typename T> unsigned long Array<T>::howManyArrays = 0;

// **************************************
// ******* USEFUL DEBUG FUNCTIONS *******
// **************************************
void loadIntArrayFromFile(Array<int> *pArray, string fileName);

#endif

