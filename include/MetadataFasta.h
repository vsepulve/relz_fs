#ifndef _METADATA_FASTA_H
#define _METADATA_FASTA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <map>

using namespace std;
class MetadataFasta {
	
protected: 
	vector<unsigned long long> pos_text;
	vector<unsigned long long> pos_storage;
	vector<unsigned int> length_line;
	char *metadata_text;
	unsigned long long metadata_length;
	
public: 
	
	MetadataFasta();
	
	virtual ~MetadataFasta();
	
	unsigned long long filterMetadata(char *text, unsigned long long text_length);
	
	void save(fstream *writer);
	
	void load(fstream *reader);
	
	// Size in bytes of the whole metadata structure stored with save
	unsigned int size();
	
};







#endif //_METADATA_FASTA_H





