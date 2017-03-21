#ifndef _BLOCK_HEADERS_FACTORY_H
#define _BLOCK_HEADERS_FACTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "BlockHeaders.h"
#include "BlockHeadersRelz.h"
#include "BytesReader.h"

using namespace std;

class BlockHeadersFactory{

private: 

public: 

	BlockHeadersFactory();
	
	~BlockHeadersFactory();
	
	// Lee el tipo
	// Construye un BlockHeaders adecuado
	// Lo carga (headers->load)
	// Lo retorna
	static BlockHeaders *load(fstream *reader);
	
	// Igual al anterior, pero a partir del nombre del archivo
	static BlockHeaders *load(const char *file_name);
	
	// Igual al anterior, pero a partir un BytesReader
	static BlockHeaders *load(BytesReader *reader);
	
	// Verifica el tipo del header
	// Escribe el tipo
	// guarda headers en el escritor
	static void save(BlockHeaders *headers, fstream *writer);
	
	// Retorna el numero de bytes de type
	static unsigned int typeSize();
	
};







#endif //_BLOCK_HEADERS_FACTORY_H





