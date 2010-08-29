#include <iostream>		// iostream header: allows input/output functions
#include <string.h>		// string functions (strcpy, etc)
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <cstdlib>
#include "hashTClass.h"
using namespace std;

unsigned long hashT::howManyHashTs = 0;

// *********** CONSTRUCTORS AND DESTRUCTORS ***********
hashT::hashT() {
	this->size = 0;
	this->table = NULL;
	this->load_factor = 0;
	this->max_load = 0;
	howManyHashTs ++;
}
hashT::hashT(unsigned long size, float max_load){
	// SIZE CANNOT BE A MULTIPLE OF 33!
	if(size%33 == 0) {
		size++; printf("\n\nWARNING, hash size cannot be a factor of 33 for djb2 hash function\n");
		printf("--> Setting size instead to %d\n\n\n",size);
	}
	this->size = size;
	this->max_load = max_load; this->load_factor = 0;
	this->table = new lList*[size]; // create an array of linked-list pointers
	for(unsigned long i = 0; i<size; i++) {this->table[i] = NULL;} // set table entries to null
	howManyHashTs ++;
}
hashT::~hashT() {
	for(unsigned long i = 0; i<this->size; i++) {this->table[i]->deleteList();} // delete all list entries
	delete[] this->table;
	howManyHashTs --;
}
// *********** REFERENCE AND MODIFIER FUNCTIONS ***********
void hashT::addKV(lList* newKV) {
	if(this == NULL) {return;}
	if(this->table == NULL || newKV == NULL) { return; }	
	unsigned long curHash = this->calcHash(newKV->getKey());
	this->table[curHash] = this->table[curHash]->insertOrder(newKV,0,1); // insert in low to high key order
	// recalculate load
	this->load_factor = (this->load_factor*((float)(this->size)) + 1)/((float)this->size); // (old_load*size + 1)/size
	if(this->load_factor > this->max_load) {
		this->rehash();
	}
	return; // we didn't rehash
}
void hashT::addKV(lList ** table, lList * newKV, unsigned long size) {
	if(this == NULL) {return;}
	if(this->table == NULL || newKV == NULL) { return; }	
	unsigned long curHash = this->calcHash(newKV->getKey());
	table[curHash] = table[curHash]->insertOrder(newKV,0,1); // insert in low to high key order
	// THIS IS USED ONLY WHEN REHASHING 
	// --> DON'T NEED TO CHECK OR RECALCULATE LOAD FACTOR (DONE HIGHER UP)
}
void hashT::rehash() {
	if(this == NULL) {return;}
	if(this->table == NULL) { return; }	
	unsigned long newSize = this->size*2; // Avoid redundant calculations
	printf("--> Loadfactor (%2.4f) is too high (max=%2.4f): rehashing to hashT size %d\n",
			this->load_factor, this->max_load, newSize);
	// MAKE SPACE FOR NEW TABLE AND SET POINTERS TO NULL
	lList** newTable = new lList*[newSize]; // Make a new hash table
	for(unsigned long j = 0; j<newSize; j++) { newTable[j]=NULL; }
	// Just iterate through every element hashT, and put it into the new hashT
	float newLoad = 0; unsigned long index = 0;
	for(unsigned long i = 0; i<this->size;i++) {
		index = 0;
		for(lList* ptemp = this->table[i]; ptemp != NULL ; /*no update */ ) {
			this->table[i] = ptemp->getNext(); // store it because we will loose it
			ptemp->setNext(NULL); ptemp->setPrev(NULL);
			this->addKV(newTable,ptemp,newSize);
			ptemp = this->table[i];
			index++;
		}
		newLoad = (newLoad*((float)newSize) + ((float)index))/((float)newSize); // (old_load*size + amount_added)/size
		this->table[i] = NULL;
	}

	delete[] this->table;
	this->table = newTable;
	this->load_factor=newLoad;
	this->size = newSize;
}
void hashT::printHash(unsigned long length) {
	if(this == NULL) {return;}
	if(this->table == NULL) { return; }	
	// Just iterate through every element in the hashT, printing as we go
	unsigned long index = 0;
	for(unsigned long i = 0; i<this->size;i++) {
		for(lList* ptemp = this->table[i]; ptemp != NULL && (length<0 || index<length); ptemp = ptemp->getNext()) {
			if(ptemp->getKey() == NULL){ printf("%d: bucket %s, key = NULL, value = %d\n",index,i,ptemp->getValue());
			} else { printf("%d: bucket %d, key = %s, value = %d\n",index,i,ptemp->getKey(),ptemp->getValue()); }
			index ++;
		}
	}
}
void hashT::printHashBucket(unsigned long length, unsigned long bucketNo) {
	if(this == NULL) {return;}
	if(this->table == NULL) { return; }	
	if(bucketNo < (this->size-1) && bucketNo >= 0) {
		// Just iterate through every element in the hashT, printing as we go
		unsigned long index = 0;
		for(lList* ptemp = this->table[bucketNo]; ptemp != NULL && (length<0 || index<length); ptemp = ptemp->getNext()) {
			if(ptemp->getKey() == NULL){ printf("%d: bucket %d, key = NULL, value = %d\n",index,bucketNo,ptemp->getValue());
			} else { printf("%d: bucket %d, key = %s, value = %d\n",index,bucketNo,ptemp->getKey(),ptemp->getValue()); }
			index ++;
		}
	}
}
void hashT::loadHashTFromFile(string fileName) {
	if(this == NULL) {return;}
	if(this->table == NULL) { return; }	
	char* cstr_word_key; string word; lList* ptemp = NULL; int value;
	cout << "Processing " << fileName << "...\n";

	// open the file if possible
	ifstream file; file.open(fileName.c_str());
	if (!file){
		cout << "Error in openening file";
		return;
	}
	// Parse into words and place key:value into the hash table (getline() gives lines, >> gives words)
	while(!file.eof()) { // Read line by line
		file >> word; // key
		cstr_word_key = new char [word.size()+1]; 	
		if(cstr_word_key == NULL) {printf("Dynamic memory allocation in buildList failed!\n"); return; }
		strcpy (cstr_word_key, word.c_str()); // strcpy to make it not const
		file >> word; // value
		sscanf(word.c_str(),"%d",&value);
		ptemp = new lList(cstr_word_key,value);
		if(ptemp == NULL) {printf("Dynamic memory allocation in buildList failed!\n"); return; }
		this->addKV(ptemp);
	}
	return;
}
lList* hashT::findHash(char * key) {
	if(this == NULL) {return NULL;}
	if(this->table == NULL || key == NULL) { return NULL; }	
	unsigned long curHash = this->calcHash(key);
	return this->table[curHash]->findKey(key);
}
unsigned long hashT::getTableLength() {
	unsigned long hashTLength = 0;
	if(this == NULL) {return -1;}
	if(this->table == NULL) { return -1; }

	for(int i = 0;i<size;i++) {
		hashTLength += this->table[i]->length();
	}
	return hashTLength;
	//return (load_factor*size); // load_factor is just the AVERAGE number of lLists per bucket
}
// *********** INTERNAL FUNTIONS ***********
unsigned long hashT::calcHash(char *key) { // djb2 --> Very general: Dan Bernstein	
	if(this == NULL) {return NULL;}
	if(key == NULL) { return NULL; }	
	unsigned long retHash = 0;		   // http://www.cse.yorku.ca/~oz/hash.html
	unsigned long cur_char;
	while(cur_char = *key++) { // while the pointer is not NULL
		retHash = ((retHash<<5) + retHash) + cur_char; // hash*33 + c --> 33 is magic number
	}
	return retHash%(this->size);
}
