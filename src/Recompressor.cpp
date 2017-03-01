#include "Recompressor.h"

Recompressor::Recompressor()
: Compressor()
{
}

Recompressor::Recompressor(const char *_master_file, CoderBlocks *_coder, DecoderBlocks *_decoder)
: Compressor(_master_file, _coder, _decoder, NULL)
{
	cout<<"Recompressor - Inicio\n";
	
	cout<<"Recompressor - Fin\n";
}

Recompressor::~Recompressor(){
	if(coder != NULL){
		delete coder;
		coder = NULL;
	}
	if(decoder != NULL){
		delete decoder;
		decoder = NULL;
	}
	if(master_file != NULL){
		delete [] master_file;
		master_file = NULL;
	}
}

bool Recompressor::recompress(const char *bytes, unsigned int n_bytes, unsigned int n_threads){
	//Verificacion de seguridad
	if(coder == NULL || decoder == NULL || bytes == NULL || n_bytes < 1 || master_file == NULL || strlen(master_file) < 1){
		cerr<<"Recompressor::compress - Datos incorrectos\n";
		return false;
	}
	lock_guard<mutex> lock(mutex_interno);
	//Borrado de datos (opcional)
	//Compresion
	if( ! realCompress(bytes, n_bytes, n_threads) ){
		return false;
	}
	//Ajustes para descompresion
	//Notar que en el recoder NO estoy usando directamente el decoder (pues no implementa metodos de descompresion)
	//Es posible que esta funcionalidad sea, de algun modo, pasada a Compressor (o que los descompresores hereden de este)
	reloadDecoder();
	return true;
}

void thread_recompress(Recompressor::ThreadCompressData *data) {
	
	//Verificacion de Seguridad
	if( data == NULL
		|| strlen(data->file_headers) == 0
		|| strlen(data->file_data) == 0
		|| data->block_size == 0
		|| data->coder == NULL
		|| data->decoder == NULL
		|| data->shared_pos == NULL
		|| data->shared_mutex == NULL
		|| data->bytes_lectura == NULL
		|| data->vector_bytes_headers == NULL
		|| data->vector_bytes_data == NULL
		|| data->vector_thread_block == NULL 
		){
		cerr<<"Recompressor::thread_compress - Error en Datos\n";
		return;
	}
	
	//Datos compartidos (punteros)
	//Coder para ESTE thread (copia del original)
	CoderBlocks *coder = data->coder->copy();
//	DecoderBlocks *decoder = data->decoder->copy();
	DecoderBlocks *decoder = data->decoder->copyBytes(data->bytes_lectura);
	unsigned int *shared_pos = data->shared_pos;
	mutex *shared_mutex = data->shared_mutex;
//	vector< pair<char*, unsigned int> > *lista_textos = data->lista_textos;
	vector<unsigned int> *vector_bytes_headers = data->vector_bytes_headers;
	vector<unsigned int> *vector_bytes_data = data->vector_bytes_data;
	vector<unsigned int> *vector_thread_block = data->vector_thread_block;
	
	shared_mutex->lock();
	cout<<"Recompressor::Thread ["<<data->id<<"] - Inicio\n";
	shared_mutex->unlock();
	
	//Archivos del thread
	cout<<"Recompressor::Thread ["<<data->id<<"] - Creando file_headers y file_data del thread\n";
	fstream file_headers(data->file_headers, fstream::trunc | fstream::binary | fstream::out);
	fstream file_data(data->file_data, fstream::trunc | fstream::binary | fstream::out);
	
	if( (! file_headers.good()) || (! file_data.good()) ){
		shared_mutex->lock();
		cerr<<"Recompressor::Thread ["<<data->id<<"] - Error al abrir archivos\n";
		shared_mutex->unlock();
		return;
	}
	
//	cout<<"Recompressor::Thread ["<<data->id<<"] - Preparando buffers\n";
	unsigned int buffer_size = coder->codingBufferSize(data->block_size);
	//Se crea este buffer en lugar de dejarlo interno en coder porque puede ser bastante grande
	char *full_buffer = new char[ buffer_size ];
	
	unsigned int block = 0;
	unsigned int proc_blocks = 0;
	
	char *text = new char[ data->block_size + 1 ];
	unsigned int text_size = 0;
	unsigned int bytes_headers = 0;
	unsigned int bytes_data = 0;
	
	while(true){
		
		shared_mutex->lock();
		block = (*shared_pos)++;
		cout<<"Recompressor::Thread ["<<data->id<<"] - Procesando block "<<block<<" (salir? "<<(block >= data->n_blocks)<<")\n";
		shared_mutex->unlock();
		if(block >= data->n_blocks){
			break;
		}
		
//		text = lista_textos->at(block).first;
//		text_size = lista_textos->at(block).second;
		//descomprimir el bloque adecuado en text
		text_size = decoder->decodeBlock(block, text);
		
		bytes_headers = 0;
		bytes_data = 0;
		cout<<"Recompressor::Thread ["<<data->id<<"] - coder->codeBlock...\n";
		coder->codeBlock(text, text_size, &file_headers, &file_data, bytes_headers, bytes_data, full_buffer);
		
		(*vector_bytes_headers)[block] = bytes_headers;
		(*vector_bytes_data)[block] = bytes_data;
		(*vector_thread_block)[block] = data->id;
		++proc_blocks;
		
	}
	
	file_headers.close();
	file_data.close();
	delete [] full_buffer;
	delete [] text;
	delete coder;
	delete decoder;
	
	shared_mutex->lock();
	cout<<"Recompressor::Thread ["<<data->id<<"] - Fin ("<<proc_blocks<<" bloques procesados)\n";
	shared_mutex->unlock();
	
	return;
}

bool Recompressor::realCompress(const char *bytes, unsigned int total_bytes, unsigned int n_threads){
	
	cout<<"Recompressor::realCompress - Inicio (en \""<<master_file<<"\", \""<<total_bytes<<"\" bytes, "<<n_threads<<")\n";
	
	/*
	unsigned int text_length = 0;
	vector< pair<unsigned int, unsigned int> > *lowcase_runs = new vector< pair<unsigned int, unsigned int> >();
	vector<unsigned int> *nl_pos = new vector<unsigned int>();
	char *text = NULL;
	if( filter != NULL ){
		text = filter->readText(in_file, text_length, lowcase_runs);
		text_length = filter->filterNewLines(text, text_length, nl_pos);
	}
	if( text == NULL ){
		cerr<<"Recompressor::realCompress - Error en carga de texto\n";
		delete lowcase_runs;
		delete nl_pos;
		return false;
	}
	
//	cout<<"Recompressor::realCompress - Texto: \""<<text<<"\" ("<<text_length<<")\n";
	
	unsigned int n_blocks = text_length / block_size;
	if( n_blocks * block_size < text_length ){
		++n_blocks;
	}
	
	vector< pair<char *, unsigned int> > lista_textos;
	
	unsigned int stored_text = 0;
	unsigned int largo_local = 0;
	for(unsigned int block = 0; block < n_blocks; ++block){
		if( text_length - stored_text < block_size ){
			largo_local = text_length - stored_text;
		}
		else{
			largo_local = block_size;
		}
		lista_textos.push_back(pair<char *, unsigned int>(text + stored_text, largo_local));
		stored_text += largo_local;
	}
	*/
	
	BytesReader lector_bytes( bytes );
	BlockHeaders *headers_lectura = BlockHeadersFactory::load( &lector_bytes );
	if( headers_lectura == NULL ){
		cerr<<"Recompressor::realCompress - Error en carga de headers\n";
		return false;
	}
	
	unsigned int text_length = headers_lectura->getTextSize();
	unsigned int block_size = headers_lectura->getBlockSize();
	unsigned int n_blocks = headers_lectura->getNumBlocks();
	
	vector<std::thread> compress_threads;
	//mutex para acceso concurrente de los threads
	mutex shared_mutex;
	//posicion del proximo bloque a ser tomado (por algun thread)
	unsigned int shared_pos = 0;
	//Tamaño en bytes de headers/data para cada bloque
	vector<unsigned int> vector_bytes_headers(n_blocks);
	vector<unsigned int> vector_bytes_data(n_blocks);
	//id del thread que procesa cada bloque
	vector<unsigned int> vector_thread_block(n_blocks);
	
	ThreadCompressData datos_threads[n_threads];
	for(unsigned int i = 0; i < n_threads; ++i){
	
		datos_threads[i].id = i;
		sprintf(datos_threads[i].file_headers, "%s.%d.headers", master_file, i);
		sprintf(datos_threads[i].file_data, "%s.%d.data", master_file, i);
		
		datos_threads[i].n_blocks = n_blocks;
		datos_threads[i].block_size = block_size;
		
		datos_threads[i].coder = coder;
		datos_threads[i].decoder = decoder;
		datos_threads[i].shared_pos = &shared_pos;
		
		datos_threads[i].shared_mutex = &shared_mutex;
//		datos_threads[i].lista_textos = &lista_textos;
		datos_threads[i].bytes_lectura = bytes;
		datos_threads[i].vector_bytes_headers = &vector_bytes_headers;
		datos_threads[i].vector_bytes_data = &vector_bytes_data;
		datos_threads[i].vector_thread_block = &vector_thread_block;
		
	}
	
	cout<<"Recompressor::realCompress - Iniciando Threads de Compresion\n";
	NanoTimer timer;
	for(unsigned int i = 0; i < n_threads; ++i){
		compress_threads.push_back( std::thread(thread_recompress, &(datos_threads[i]) ) );
	}
	for(unsigned int i = 0; i < n_threads; ++i){
		compress_threads[i].join();
	}
//	lista_textos.clear();
//	delete [] text;
	cout<<"Recompressor::realCompress - Threads terminados en "<<timer.getMilisec()<<" ms, iniciando Merge\n";
	
	//Merge en el archivo maestro
	
	//Total de bytes leidos hasta el momento de los archivos de headers/data de cada thread
	unsigned int bytes_headers_thread[n_threads];
	unsigned int bytes_data_thread[n_threads];
	memset(bytes_headers_thread, 0, n_threads * sizeof(int));
	memset(bytes_data_thread, 0, n_threads * sizeof(int));
	
	//Escritura de headers en el master (headers es del tipo de decoder->headers)
	BlockHeaders *headers = decoder->getNewHeaders(text_length, block_size, headers_lectura->getMetadata() );
	headers_lectura->setMetadata(NULL);
	delete headers_lectura;
	
	//Buffer para la copia
	unsigned int buffer_size = 1024*1024;
	char *buff_lectura = new char[buffer_size];
	fstream lector;
	
	for(unsigned int block = 0; block < n_blocks; ++block){
		unsigned int thread_id = vector_thread_block[block];
		
		//Cargar el header particular
		//Los bytes son para asegurar la lectura del header correcto
		unsigned int bytes_headers = vector_bytes_headers[block];
		//Lectura de archivo del thread
		lector.open(datos_threads[thread_id].file_headers, fstream::binary | fstream::in);
		if( (! lector.good()) || (! lector.is_open()) ){
			cerr<<"Recompressor::realCompress - Error en lectura de \""<<datos_threads[thread_id].file_headers<<"\"\n";
			break;
		}
		lector.seekg(bytes_headers_thread[thread_id], lector.beg);
		headers->loadBlock(&lector, bytes_headers);
		lector.close();
		bytes_headers_thread[thread_id] += bytes_headers;
	}
	
	//Escritura de data en el master (luego de la escritura puede borrarse el objeto headers)
	fstream escritor(master_file, fstream::trunc | fstream::binary | fstream::out);
	headers->prepare();
//	headers->save(&escritor);
	BlockHeadersFactory::save(headers, &escritor);
	delete headers;
	for(unsigned int block = 0; block < n_blocks; ++block){
		unsigned int thread_id = vector_thread_block[block];
		
		//Copiar data en archivo del thread desde bytes_data_thread al master
		unsigned int bytes_data = vector_bytes_data[block];
		if(bytes_data + 1 > buffer_size){
			delete [] buff_lectura;
			buffer_size = bytes_data + 1;
			buff_lectura = new char[buffer_size];
		}
		//Lectura de archivo del thread
		lector.open(datos_threads[thread_id].file_data, fstream::binary | fstream::in);
		if( (! lector.good()) || (! lector.is_open()) ){
			cerr<<"Recompressor::realCompress - Error en lectura de \""<<datos_threads[thread_id].file_data<<"\"\n";
			break;
		}
		lector.seekg(bytes_data_thread[thread_id], lector.beg);
		lector.read(buff_lectura, bytes_data);
		lector.close();
		//escritura de bytes
		escritor.write(buff_lectura, bytes_data);
		bytes_data_thread[thread_id] += bytes_data;
		
	}
	
	delete [] buff_lectura;
	escritor.close();
	
	//Eliminacion de archivos de threads
	for(unsigned int i = 0; i < n_threads; ++i){
		remove(datos_threads[i].file_headers);
		remove(datos_threads[i].file_data);
	}
	cout<<"Recompressor::realCompress - Fin\n";
	return true;
	
}









