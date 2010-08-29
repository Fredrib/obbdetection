// File:		hashset.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// Custom hash set implimentation --> I didn't like hash_set functions!  Too slow!
// Basically just a hash table for key values, without the values.

// NOTE: For speed, The table is just a flat array of integers!  This means that the
// number of elements that can go in a bucket is capped (at maxbucketLength).  When
// a bucket fills, we rehash and try again continuously.

// THIS IS A SHITTY HASH TABLE IMPLIMENTATION...  I AM NOT HAPPY WITH THIS.

#include "main.h"

#ifndef hashset_int_h
#define hashset_int_h

class hashset_int {
public:
	// ***** CONSTRUCTORS AND DESTRUCTORS *****
	hashset_int(int numBuckets_in, int maxbucketLength_in, double maxLoad_in);
	~hashset_int();
	// NO COPY CONSTRUCTOR! --> DO LATER IF NECESSARY

	// ***** REFERENCE AND MODIFIER FUNCTIONS *****
	int GetSize(){return numBuckets;};											//O(1)
	double GetLoad(){return loadFactor;};										//O(1)
	void AddKey(int * newKey);													//O(M), where M is the number of entries in the bucket
	int * Rehash(int * retsize); // HASH SET WILL AUTO REHASH!					//O(N), where N is number of entries
	bool FindHash(int * key);													//O(1) - whoot!
	void ReturnArray(int * returnSize, int ** retArray);						//O(N)

private:
	// Internal Variables and functions
	int numBuckets;
	int elementsInArray;
	int maxBucketLength;
	int * table; // HASH TABLE IS A FLAT ARRAY!
	double loadFactor;
	double maxLoad;
	int CalcHash(int * key, int size); // NEED HASH FUNCTION
	void _debugIterateHash(int * newTable, int size, int bucketLength);
	void AddKeyToTable(int * newTable, int * newKey, int size); // Internal used in Rehash
};

#endif