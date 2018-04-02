#ifndef _CODER_BLOCKS_H
#define _CODER_BLOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "NanoTimer.h"

//#include "ReferenceIndex.h"

using namespace std;

class CoderBlocks{

protected: 
	
public: 
	
	CoderBlocks();
	
	virtual ~CoderBlocks();
	
//	virtual void codeBlock(const char *text, unsigned int text_size, fstream *file_headers, fstream *file_data, unsigned int &bytes_headers, unsigned int &bytes_data, const ReferenceIndex *referencia, char *full_buffer);
	virtual void codeBlock(const char *text, unsigned int text_size, fstream *file_headers, fstream *file_data, unsigned int &bytes_headers, unsigned int &bytes_data, char *full_buffer);
	
	//Retorna el tamaño en bytes del buffer necesario para codeBlock
	virtual unsigned int codingBufferSize(unsigned int block_size);
	
	//Retorna un puntero a un nuevo Coder del mismo tipo
	//Adicionalmente podria copiar otros datos internos (pero el nuevo objeto debe ser independiente)
	//El llamador debe encargarse de borrar la copia
	virtual CoderBlocks *copy() const;
	
};







#endif //_DECODER_BLOCKS_H





