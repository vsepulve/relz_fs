#ifndef _DECODER_BLOCKS_COMPACT_H
#define _DECODER_BLOCKS_COMPACT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "BitsUtils.h"
#include "BlockHeaders.h"
#include "LengthsCoderBlocks.h"
#include "PositionsCoderBlocks.h"
#include "CompactSequence.h"

using namespace std;

class DecoderBlocksCompact{

private: 
	BitsUtils utils;
	BlockHeaders *headers;
	LengthsCoderBlocks *len_coder;
	PositionsCoderBlocks *pos_coder;
	CompactSequence *texto;
	
	//Buffers para almacenar el bloque actual descomprimido
	unsigned int buff_size;
	unsigned int *buff_pos;
	unsigned int *buff_len;
	unsigned int cur_block;
	
	unsigned int n_blocks;
	unsigned int cur_block_ini;
	unsigned int cur_block_factores;
	unsigned int block_size;
	
	void decodeBlock(unsigned int block_pos);
	
public: 
	DecoderBlocksCompact();
	DecoderBlocksCompact(const char *master_file, CompactSequence *_texto);
	virtual ~DecoderBlocksCompact();
	
	//Decodifica el texto
	//Extrae length caracteres desde pos_ini y lo almacena en buff agregando un 0 al final
	//Asume que el buff tiene al menos (length + 1) chars de espacio.
	//Retorna el numero de caracteres leidos (puede ser menor a length al final del string)
	//En caso de pos_ini mayor al final, simplemente retorna 0.
	unsigned int decodeText(unsigned int pos_ini, unsigned int length, char *buff);
	
	void testBlocks();
	
};







#endif //_DECODER_BLOCKS_H





