#ifndef _RECOMPRESSOR_H
#define _RECOMPRESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <thread>
#include <mutex>
#include <vector>

#include "Compressor.h"
#include "BytesReader.h"
#include "BlockHeaders.h"
#include "BlockHeadersFactory.h"

using namespace std;

class Recompressor : public Compressor{

protected: 
	
//	char *master_file;
//	CoderBlocks *coder;
//	DecoderBlocks *decoder;
//	mutex mutex_interno;
//	TextFilter *filter;
	
//	static const bool adjust_text = true;
	
	// Compresion efectiva, separada para simplificar ligeramente el codigo
	// Retorna true en exito, false en caso de fallo
	bool realCompress(const char *bytes, unsigned int n_bytes, unsigned int n_threads = 4);
	
public: 

	class ThreadCompressData{
	public:
	
		ThreadCompressData(){
			id = 0;
			file_headers[0] = 0;
			file_data[0] = 0;
			n_blocks = 0;
			block_size = 0;
			coder = NULL;
			decoder = NULL;
			shared_pos = NULL;
			shared_mutex = NULL;
//			lista_textos = NULL;
			vector_bytes_headers = NULL;
			vector_bytes_data = NULL;
			vector_thread_block = NULL;
		}
		
		~ThreadCompressData(){
		}
		
		//Datos del thread
		unsigned int id;
		//Ojo con el largo del nombre de archivo
		char file_headers[128];
		char file_data[128];
		
		//Datos comunes (constantes)
		unsigned int n_blocks;
		unsigned int block_size;
		
		//Datos compartidos (punteros)
		const CoderBlocks *coder;
		const DecoderBlocks *decoder;
		unsigned int *shared_pos;
		mutex *shared_mutex;
//		vector< pair<char*, unsigned int> > *lista_textos;
		const char *bytes_lectura;
		vector<unsigned int> *vector_bytes_headers;
		vector<unsigned int> *vector_bytes_data;
		vector<unsigned int> *vector_thread_block;
		
	};

	// Constructor vacio
	Recompressor();
	
	// Constructor principal de la clase Recompressor
	// _master_file: ruta asociada al archivo (hacia donde se comprime o desde donde se descomprime, guarda una copia del string)
	// _coder: CoderBlocks que se usara para la compresion y escritura. El Recompressor toma posecion del objeto y lo borrara al terminar
	// _decoder: DecoderBlocks que se usara para descompresion y lectura. El Recompressor tambien toma posecion de este objeto.
//	Recompressor(const char *_master_file, CoderBlocks *_coder, DecoderBlocks *_decoder, TextFilter *_filter = NULL);
	Recompressor(const char *_master_file, CoderBlocks *_coder, DecoderBlocks *_decoder);
	
	virtual ~Recompressor();
	
	// Recompresion de los datos ya comprimidos en bytes (total_bytes chars)
	// REEMPLAZANDO el master_file asociado a este compressor
	// Reusa los metadatos del BlockHeaders contenido en bytes, incluyendo block_size 
	// Retorna true en exito, false en caso de fallo
	virtual bool recompress(const char *bytes, unsigned int total_bytes, unsigned int n_threads = 4);
	
};







#endif //_RECOMPRESSOR_H





