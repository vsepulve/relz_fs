#ifndef _RECODER_BLOCKS_H
#define _RECODER_BLOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "NanoTimer.h"
#include "BitsUtils.h"
#include "BlockHeaders.h"
#include "LengthsCoderBlocks.h"
#include "PositionsCoderBlocks.h"

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"
#include "ReferenceIndexRR.h"

#include "DecoderBlocksBytes.h"
#include "BytesReader.h"

using namespace std;

class RecoderBlocks{

private: 
	BitsUtils utils;
	ReferenceIndex *referencia;
	
public: 
	
	class ThreadCompressData{
	public:
		ThreadCompressData(){}
		~ThreadCompressData(){
		}
		
		//Datos del thread
		unsigned int id;
		
		//Datos comunes (constantes)
		unsigned int n_blocks;
		unsigned int block_size;
		
		//Datos compartidos (punteros)
		ReferenceIndex *referencia;
		unsigned int *shared_pos;
		mutex *shared_mutex;
//		vector< pair<char*, unsigned int> > *lista_textos;
		//En lugar de lista de texto por bloque, esta version incluye los bytes de lectura
		//Cara thread creara su propio decodificador in-memory para leer esos bytes extrayendo el texto de cada bloque
		char *bytes_lectura;
		vector<unsigned int> *vector_bytes_pos;
		vector<unsigned int> *vector_bytes_len;
		vector<unsigned int> *vector_n_factores;
		vector<unsigned int> *vector_thread_block;
		
	};
	
	RecoderBlocks(ReferenceIndex *_referencia);
	virtual ~RecoderBlocks();
	
//	void compress(const char *archivo_entrada, const char *archivo_salida, unsigned int n_threads = 4, unsigned int block_size = 1000000);
	void compress(char *bytes, unsigned int total_bytes, const char *archivo_salida, unsigned int n_threads = 4);
	
};







#endif //_DECODER_BLOCKS_H





