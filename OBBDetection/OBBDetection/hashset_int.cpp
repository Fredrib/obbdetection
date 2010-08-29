// File:		hashset.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

#include "main.h"

// *********** CONSTRUCTORS AND DESTRUCTORS ***********
hashset_int::hashset_int(int numBuckets_in, int maxbucketLength_in, double maxLoad_in)
{
	numBuckets = numBuckets_in;
	maxBucketLength = maxbucketLength_in;
	maxLoad = maxLoad_in;
	table = new int[numBuckets*(maxBucketLength+1)]; // first element in the bucket is the number of elements
	for(int i = 0; i < numBuckets; i ++)
		table[i*(maxbucketLength_in+1)] = 0; // set all bucket sizes to zero
	loadFactor = 0;
	elementsInArray = 0;
}
hashset_int::~hashset_int() 
{
	delete[] table;
	table = NULL;
}
// *********** REFERENCE AND MODIFIER FUNCTIONS ***********
void hashset_int::AddKey(int * newKey) {
	AddKeyToTable(table, newKey, numBuckets);
}
int * hashset_int::Rehash(int * retsize) {
	if(table == NULL)
		throw std::runtime_error("hashset_int::Rehash: Hash table is null!");

	int newSize = numBuckets*2; // Avoid redundant calculations
	* retsize = newSize;
	//// MAKE SPACE FOR NEW TABLE AND SET POINTERS TO NULL
	int * newTable = new int[newSize*(maxBucketLength+1)]; // first element in the bucket is the number of elements
	for(int i = 0; i < newSize; i ++)
		newTable[i*(maxBucketLength+1)] = 0; // set all bucket sizes to zero

	// Start number of elements at zero.
	elementsInArray = 0;

	// Just iterate through every element hashset_int, and put it into the new hashset_int
	for(int i = 0; i < numBuckets; i++) 
	{
		int bucketSize = table[i*(maxBucketLength+1)];
		for(int j = 0; j < bucketSize; j ++) 
		{
			AddKeyToTable(newTable,&table[i*(maxBucketLength+1) + j + 1],newSize);
		}
	}
	loadFactor = (double) elementsInArray / (double) newSize;
	delete[] table;
	table = newTable;
	numBuckets = newSize;
	return table;
	throw std::runtime_error("HASH SET WAS REHASHED --> DON'T WANT TO DO THIS FOR NOW (SLOW)... CONSIDER LARGER BUCKET SIZE");
}
void hashset_int::AddKeyToTable(int * newTable, int * newKey, int size) {
	if(newTable == NULL)
		throw std::runtime_error("hashset_int::AddKey: Hash table is null!");
	if(newKey == NULL) 
		throw std::runtime_error("hashset_int::AddKey: Attempting to insert a null entry into hashtable!");
	
	int curHash = CalcHash(newKey, size);
	int bucketSize = newTable[curHash*(maxBucketLength+1)];

	for(int i = 0; i < bucketSize; i ++)
	{
		if(newTable[curHash*(maxBucketLength+1) + i + 1] == *newKey)
			return; // Value already exists in hash table
	}

	while(bucketSize >= maxBucketLength) // Keep rehashing until bucket isn't full
	{
		_debugIterateHash(newTable, size, maxBucketLength);
		newTable = Rehash(&size);	// I SUSPECT SOMETHING IS WRONG IN HERE!!! 12/31/2009
		_debugIterateHash(newTable, size, maxBucketLength);
		curHash = CalcHash(newKey, size);
		bucketSize = newTable[curHash*(maxBucketLength+1)];
	}

	newTable[curHash*(maxBucketLength+1) + bucketSize + 1] =  *newKey;
	newTable[curHash*(maxBucketLength+1)] += 1;
	elementsInArray += 1;

	loadFactor = (double) elementsInArray / (double) numBuckets;
	while(loadFactor >= maxLoad)
		newTable = Rehash(&size);
}
bool hashset_int::FindHash(int * key) {
	if(table == NULL)
		throw std::runtime_error("hashset_int::FindHash: Hash table is null!");
	if(key == NULL) 
		throw std::runtime_error("hashset_int::FindHash: Attempting to insert a null entry into hashtable!");

	int curHash = CalcHash(key, numBuckets);
	int bucketSize = table[curHash*(maxBucketLength+1)];
	for(int i = 0; i < bucketSize; i ++)
		if(table[curHash*(maxBucketLength+1) + i + 1] == *key)
			return true; // Value already exists in hash table

	return false; // Value doesn't exist
}
void hashset_int::ReturnArray(int * returnSize, int ** retArray)
{
	*returnSize = elementsInArray; // This is scary but it should be an integer

	// Space is already allocated, just overwrite old space.
//	int * retval = new int[elementsInArray];

	int curIndex = 0;
	for(int i = 0; i < numBuckets; i++) 
	{
		int bucketSize = table[i*(maxBucketLength+1)];
		for(int j = 0; j < bucketSize; j ++) 
		{
			(*retArray)[curIndex] = table[i*(maxBucketLength+1) + j + 1];
			curIndex++;
			if(*returnSize < curIndex)
				throw std::runtime_error("hashset_int::ReturnArray index ran out of return size");
		}
	}
}
// *********** INTERNAL FUNTIONS ***********
// http://www.concentric.net/~Ttwang/tech/inthash.htm
int hashset_int::CalcHash(int * key, int size) { 
	if(key == NULL) 
		throw std::runtime_error("hashset_int::CalcHash: Attempting to calculate hash function for a null entry!");
	return *key % size; // Lazy hash function --> This is ok because we're storing vertex indices
	// which are likely to be grouped in contiguous chunks... I hope ;-)
}
void hashset_int::_debugIterateHash(int * newTable, int size, int bucketLength)
{
	int curKey = 0;
	int curBucketSize = 0;
	for(int j = 0; j < size; j ++)
	{
		curBucketSize = newTable[j*(bucketLength+1)]; // First integer is the size of the current bucket
		for(int i = 0; i < curBucketSize; i ++)
		{
			curKey = newTable[j*(bucketLength+1) + i + 1];
			int temp = curKey;
		}
	}
}