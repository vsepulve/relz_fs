#ifndef _COMPACT_SEQUENCE_H
#define _COMPACT_SEQUENCE_H

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

using namespace std;

class CompactSequence{

private:
	//largo original del texto
	unsigned int text_size;
	//numero de bytes real
	unsigned int n_bytes;
	//bytes
	char *bytes;

public: 
	CompactSequence();
	CompactSequence(char *_text, unsigned int _text_size);
	virtual ~CompactSequence();
	
	virtual char getChar(unsigned int position);
	virtual unsigned int getText(unsigned int position, unsigned int length, char *out);
	
	virtual void save(fstream *writer);
	virtual void load(fstream *reader);
	
};

#endif //_COMPACT_SEQUENCE_H

