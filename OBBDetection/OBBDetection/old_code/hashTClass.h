#include <iostream>		// iostream header: allows input/output functions
#include <string.h>		// string functions (strcpy, etc)
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <cstdlib>
#include "../../linked_lists/study_examples/lListClass.h";

#ifndef HASHTCLASS
#define HASHTCLASS

using namespace std;

class hashT {
public:
	// ***** CONSTRUCTORS AND DESTRUCTORS *****
	hashT();
	hashT(unsigned long size, float max_load);
	~hashT();

	// ***** REFERENCE AND MODIFIER FUNCTIONS *****
	unsigned long getSize(){return size;};										//O(1)
	float getLoad(){return load_factor;};										//O(1)
	void addKV(lList * newKV);													//O(1+N) N is length of hash bin (hopefully 1 for low load)
	void rehash();																//O(N)
	void printHash(unsigned long length = -1);									//O(N)
	void printHashBucket(unsigned long length, unsigned long bucketNo);			//O(N)
	void loadHashTFromFile(string fileName);									//>O(N) - Probably need to reballance a few times if size is small
	lList* findHash(char * key);												//O(1) - whoot!
	unsigned long getTableLength();												//O(N) --> could be order one (load*size) but this is more thorough!

	// ***** HASH SUB FUNCTIONS ***** --> MAKE PRIVATE AFTER TESTING
	unsigned long calcHash(char* key); // djb2 --> Very general: Dan Bernstein	//O(1) - + overhead

	// DEBUGGING
	static unsigned long howManyHashTs;

private:
	// Internal Variables and functions
	void addKV(lList ** table, lList * newKV, unsigned long size);				//Add to a table (not to a hash table class)
	unsigned long size;
	lList** table; // Array of pointers to linked-lists!
	float load_factor;
	float max_load;
};

#endif