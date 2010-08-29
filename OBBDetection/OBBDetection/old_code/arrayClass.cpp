#include "arraysInc.h"

// **************************************
// ******* USEFUL DEBUG FUNCTIONS *******
// **************************************
void loadIntArrayFromFile(Array<int> *pArray, string fileName) {
	string word; int value;
	cout << "Processing " << fileName.c_str() << "...\n";

	// open the file if possible
	ifstream file; file.open(fileName.c_str());
	if (!file){
		std::cout << "Error in openening file";
		return;
	}
	// Parse into words and place key:value into the hash table (getline() gives lines, >> gives words)
	while(!file.eof()) { // Read line by line
		file >> word; // value
		sscanf(word.c_str(),"%d",&value);
		pArray->add(value);
	}
	return;
};