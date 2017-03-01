#ifndef _DECODER_BLOCKS_H
#define _DECODER_BLOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "BitsUtils.h"
#include "NanoTimer.h"

#include "BlockHeaders.h"
#include "BlockHeadersFactory.h"
#include "Metadata.h"

using namespace std;

class DecoderBlocks{

protected: 
	BlockHeaders *headers;
	
public: 
	DecoderBlocks();
	DecoderBlocks(const char *master_file);
	virtual ~DecoderBlocks();
	
	//Prepara el decoder cargando todo lo necesario del archivo (headers, otros decoders, etc)
	virtual void load(const char *master_file);
	
	//Asume que el buff tiene al menos getBlockSize() + 1 bytes
	//Retorna el tamaño efectivo del bloque, getBlockSize() salvo por el ultimo
	//Omite block_pos fuera de rango (lo verifica)
	virtual unsigned int decodeBlock(unsigned int block_pos, char *buff);
	
	//Metodo directo de headers (no es necesario redefinirlo)
	virtual unsigned long long getTextSize();
	
	//Metodo directo de headers (no es necesario redefinirlo)
	virtual unsigned int getBlockSize();
	
	//Metodo directo de headers (no es necesario redefinirlo)
	virtual unsigned int getNumBlocks();
	
	//Metodo directo de headers (no es necesario redefinirlo)
	virtual BlockHeaders *getHeaders();
	
	//Este metodo retorna un NUEVO objeto headers del tipo correcto
	//Usa los argumentos en la creacion del header
	virtual BlockHeaders *getNewHeaders(unsigned long long _text_size, unsigned int _block_size, Metadata *_metadata = NULL);
	
	//Retorna un puntero a un nuevo Decoder del mismo tipo
	//Adicionalmente podria copiar otros datos internos (pero el nuevo objeto debe ser independiente)
	//El llamador debe encargarse de borrar la copia
	virtual DecoderBlocks *copy() const;
	
	virtual DecoderBlocks *copyBytes(const char *bytes) const;
	
};







#endif //_DECODER_BLOCKS_H





