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

#include <lzma.h>

#include "NanoTimer.h"
#include "BitsUtils.h"

using namespace std;
class MetadataFasta {
	
protected: 
	vector<unsigned long long> pos_text;
	vector<unsigned long long> pos_storage;
	vector<unsigned int> length_line;
	char *metadata_text;
	unsigned long long metadata_length;
	
	unsigned char* CompressWithLzma(const char *text, size_t length, int level, size_t &output_length);
//	std::string DecompressWithLzma(const std::string& in);
	char* DecompressWithLzma(const unsigned char *input, size_t length, size_t &output_length);
	
	unsigned char *compressed_buff;
	size_t compressed_bytes;
	bool already_compressed;
	
public: 
	
	MetadataFasta();
	
	virtual ~MetadataFasta();
	
	unsigned long long filterMetadata(char *text, unsigned long long text_length);
	
	void save(fstream *writer);
	
	void load(fstream *reader);
	
	// Size in bytes of the whole metadata structure stored with save
	unsigned int size();
	
	void adjustText(char *out_buff, unsigned long long pos_ini, unsigned int copied_chars, char *adjust_buffer);
	
	unsigned long long countText(unsigned long long pos);
	unsigned long long countTextBin(unsigned long long pos);
	
};







#endif //_METADATA_FASTA_H





