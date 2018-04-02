#ifndef _COMPRESSOR_SINGLE_BUFFER_H
#define _COMPRESSOR_SINGLE_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <mutex>
#include <vector>

#include "Compressor.h"

using namespace std;

class CompressorSingleBuffer : public Compressor{
	
private: 
	
//	const char *master_file;
//	CoderBlocks *coder;
//	DecoderBlocks *decoder;
//	mutex mutex_interno;
//	TextFilter *filter;
	
	unsigned int cur_block;
	unsigned int cur_block_size;
	unsigned int buffer_size;
	//Buffer interno
	char *buffer;
	//Clon del buffer para procesos de ajustes (de modo que pueda reusarse el buff de last block)
	char *adjust_buffer;
	
	//Resetea y extiende los buffers solo si new_size es mayor a buffer_size
	void prepareBuffer(unsigned int new_size);
	
public: 
	CompressorSingleBuffer();
	CompressorSingleBuffer(const char *_master_file, CoderBlocks *_coder, DecoderBlocks *_decoder, TextFilter *_filter = NULL);
	virtual ~CompressorSingleBuffer();
	
	// Extrae texto comprimido
	// Escribe length chars desde la posicion absoluta pos_ini en buff (asume al menos length + 1 chars de espacio)
	// Retorna el largo del texto escrito (igual a strlen de buff)
	unsigned int read(unsigned long long pos_ini, unsigned int length, char *out_buff);
	
	// Escribe texto recomprimiendo los bloques involucrados
	// ESTO AUN NO ESTA TOTALMENTE IMPLEMENTADO
	unsigned int write(const char *text, unsigned int length, unsigned long long pos_ini);
	
	// Descompresion completa del master_file asociado a este compressor
	// Escribe el texto descomprimido en out_file (en lineas de un cierto largo)
	// Eventualmente line_size sera removida
	// Datos como eso de ser necesarios, estaran en los metadatos de BlockHeaders del Coder/Decoder
	// Retorna true en exito, false en caso de fallo
	bool decompress(const char *out_file, unsigned int line_size = 70);
	
	// Este metodo debe recargar al decoder y dejar al compresor en estado inicial
	// Eso puede implicar resetear buffers y otras variables de estado
	void reloadDecoder(){
		if(decoder == NULL || master_file == NULL || strlen(master_file) < 1){
			cerr<<"CompressorSingleBuffer::reloadDecoder - Datos incorrectos\n";
		}
		else{
			decoder->load(master_file);
		}
		cur_block = 0xffffffff;
		cur_block_size = 0;
	}
	
};







#endif //_DECODER_BLOCKS_H





