#ifndef _DECODER_BLOCKS_BYTES_H
#define _DECODER_BLOCKS_BYTES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DecoderBlocks.h"

#include "BlockHeadersRelz.h"
#include "LengthsCoderBlocksBytes.h"
#include "PositionsCoderBlocksBytes.h"

using namespace std;

class DecoderBlocksRelzBytes : public DecoderBlocks{

private: 

	//Este es solo un puntero al texto, NO lo copia
	const char *texto_ref;
	
	LengthsCoderBlocksBytes *len_coder;
	PositionsCoderBlocksBytes *pos_coder;
	
	//Buffers para almacenar el bloque actual descomprimido
	unsigned int buff_size;
	unsigned int *buff_pos;
	unsigned int *buff_len;
	unsigned int cur_block;
	unsigned int cur_block_factores;
	
//	unsigned int n_blocks;
//	unsigned int cur_block_ini;
//	unsigned int block_size;
	
//	void decodeBlock(unsigned int block_pos);
	
	//Borra headers, coders y resetea cur_block
	//Conserva los buffers
	void deleteData();
	
	//Borra los buffers
	void deleteBuffers();
	
	//Resetea y extiende los buffers solo si new_size es mayor a buff_size
	void prepareBuffer(unsigned int new_size);
	
public: 
	//Notar que todo DecoderBlocksRelz usable DEBE construirse con un texto de referencia valido
	DecoderBlocksRelzBytes(const char *_texto_ref = NULL);
	DecoderBlocksRelzBytes(const char *bytes, const char *_texto_ref);
	virtual ~DecoderBlocksRelzBytes();
	
	virtual void load(const char *bytes);
	
	//No verifica el tamaño del buffer, asume que el llamador conoce block_size
	//Retorna el tamaño efectivo (block_size, o el resto en caso del ultimo bloque)
	virtual unsigned int decodeBlock(unsigned int block, char *buffer);
	
//	virtual unsigned int decodeBlockFactors(unsigned int block, unsigned int *buff_pos, unsigned int *buff_len);
	
	//Este metodo retorna un NUEVO objeto headers del tipo correcto
	//Usa los argumentos en la creacion del header
	virtual BlockHeaders *getNewHeaders(unsigned int _text_size, unsigned int _block_size, Metadata *_metadata = NULL);
	
};







#endif //_DECODER_BLOCKS_H





