#include "Compressor.h"

Compressor::Compressor(){
	master_file = NULL;
	coder = NULL;
	decoder = NULL;
	filter = NULL;
}

Compressor::Compressor(const char *_master_file, CoderBlocks *_coder, DecoderBlocks *_decoder, TextFilter *_filter){
	cout<<"Compressor - Inicio\n";
	master_file = new char[strlen(_master_file) + 1];
	strcpy(master_file, _master_file);
	coder = _coder;
	decoder = _decoder;
	if(decoder != NULL){
		decoder->load(master_file);
	}
	filter = _filter;
	if(filter == NULL){
		filter = new TextFilterBasic();
	}
	cout<<"Compressor - Fin\n";
}

Compressor::~Compressor(){
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
	if(filter != NULL){
		delete filter;
		filter = NULL;
	}
}
	
long long Compressor::getTextSize(){
	if(decoder != NULL){
		return decoder->getTextSize();
	}
	return 0;
}

unsigned int Compressor::read(unsigned long long pos_ini, unsigned int length, char *buff){
	cerr<<"Compressor::read - No Implementado\n";
	return 0;
	//Calcular bloques involucrados
	//Iterar por cada bloque
	//	Descomprimir el bloque
	//	Calcular limites del bloque
	//	memcpy del buffer_interno al buff salida
	//Retornar total copiado
}

unsigned int Compressor::write(const char *text, unsigned int length, unsigned long long pos_ini){
	cerr<<"Compressor::write - No Implementado\n";
	return 0;
	//Verificacion de seguridad
	//Calcular bloques afectados
	//Iterar por cada bloque afectado
	//	Descomprimir bloque en buffer
	//	Modificar el bloque
	//	Recomprimir el bloque en otro archivo
	//Ajustar el archivo maestro con bloques recomprimidos
}

bool Compressor::decompress(const char *out_file, unsigned int line_size){
	cerr<<"Compressor::decompress - No Implementado\n";
	//Verificacion de seguridad
	//Descompresion
	return false;
}

bool Compressor::compress(const char *in_file, unsigned int n_threads, unsigned int block_size, bool use_metadata){
	//Verificacion de seguridad
	//Ojo con strlen(in_file) que debe soportar 64 bits para ser valido
	if(coder == NULL || decoder == NULL || in_file == NULL || strlen(in_file) < 1 || master_file == NULL || strlen(master_file) < 1){
		cerr<<"Compressor::compress - Datos incorrectos\n";
		return false;
	}
	lock_guard<mutex> lock(mutex_interno);
	//Borrado de datos (opcional)
	//Compresion
	if( ! realCompress(in_file, n_threads, block_size, use_metadata) ){
		return false;
	}
	//Ajustes para descompresion
	reloadDecoder();
	return true;
}

void thread_compress(Compressor::ThreadCompressData *data) {
	
	//Verificacion de Seguridad
	if( data == NULL
		|| strlen(data->file_headers) == 0
		|| strlen(data->file_data) == 0
		|| data->block_size == 0
		|| data->coder == NULL
		|| data->shared_pos == NULL
		|| data->shared_mutex == NULL
		|| data->lista_textos == NULL
		|| data->vector_bytes_headers == NULL
		|| data->vector_bytes_data == NULL
		|| data->vector_thread_block == NULL 
		){
		cerr<<"Compressor::thread_compress - Error en Datos\n";
		return;
	}
	
	//Datos compartidos (punteros)
	//Coder para ESTE thread (copia del original)
	CoderBlocks *coder = data->coder->copy();
	unsigned int *shared_pos = data->shared_pos;
	mutex *shared_mutex = data->shared_mutex;
	vector< pair<char*, unsigned int> > *lista_textos = data->lista_textos;
	vector<unsigned int> *vector_bytes_headers = data->vector_bytes_headers;
	vector<unsigned int> *vector_bytes_data = data->vector_bytes_data;
	vector<unsigned int> *vector_thread_block = data->vector_thread_block;
	
	shared_mutex->lock();
	cout<<"Compressor::Thread ["<<data->id<<"] - Inicio\n";
	shared_mutex->unlock();
	
	//Archivos del thread
//	cout<<"Compressor::Thread ["<<data->id<<"] - Creando file_headers y file_data del thread\n";
	fstream file_headers(data->file_headers, fstream::trunc | fstream::binary | fstream::out);
	fstream file_data(data->file_data, fstream::trunc | fstream::binary | fstream::out);
	
	if( (! file_headers.good()) || (! file_data.good()) ){
		shared_mutex->lock();
		cerr<<"Compressor::Thread ["<<data->id<<"] - Error al abrir archivos\n";
		shared_mutex->unlock();
		return;
	}
	
	unsigned int buffer_size = coder->codingBufferSize(data->block_size);
	//Se crea este buffer en lugar de dejarlo interno en coder porque puede ser bastante grande
//	cout<<"Compressor::Thread ["<<data->id<<"] - Preparando buffer de tamaño "<<buffer_size<<"\n";
	char *full_buffer = new char[buffer_size];
	
	unsigned int block = 0;
	unsigned int proc_blocks = 0;
	
	char *text = NULL;
	unsigned int text_size = 0;
	unsigned int bytes_headers = 0;
	unsigned int bytes_data = 0;
	
	while(true){
		
		shared_mutex->lock();
		block = (*shared_pos)++;
//		cout<<"Compressor::Thread ["<<data->id<<"] - Procesando block "<<block<<" (salir? "<<(block >= data->n_blocks)<<")\n";
		shared_mutex->unlock();
		if(block >= data->n_blocks){
			break;
		}
		
		text = lista_textos->at(block).first;
		text_size = lista_textos->at(block).second;
		
		bytes_headers = 0;
		bytes_data = 0;
//		cout<<"Compressor::Thread ["<<data->id<<"] - coder->codeBlock...\n";
		coder->codeBlock(text, text_size, &file_headers, &file_data, bytes_headers, bytes_data, full_buffer);
		
		(*vector_bytes_headers)[block] = bytes_headers;
		(*vector_bytes_data)[block] = bytes_data;
		(*vector_thread_block)[block] = data->id;
		++proc_blocks;
		
	}
	
	file_headers.close();
	file_data.close();
	delete [] full_buffer;
	delete coder;
	
	shared_mutex->lock();
	cout<<"Compressor::Thread ["<<data->id<<"] - Fin ("<<proc_blocks<<" bloques procesados)\n";
	shared_mutex->unlock();
	
	return;
}

bool Compressor::realCompress(const char *in_file, unsigned int n_threads, unsigned int block_size, bool use_metadata){
	
	cout<<"Compressor::realCompress - Inicio (en \""<<master_file<<"\", desde \""<<in_file<<"\", n_threads: "<<n_threads<<", block_size: "<<block_size<<", use_metadata: "<<use_metadata<<")\n";
	unsigned long long text_length = 0;
	char *text = NULL;
	vector< pair<unsigned long long, unsigned long long> > *lowcase_runs = NULL;
	vector<unsigned long long> *nl_pos = NULL;
	if( use_metadata ){
		//Definicion para metadatos: lowcase
		lowcase_runs = new vector< pair<unsigned long long, unsigned long long> >();
		//Definicion para metadatos: newlines
		nl_pos = new vector<unsigned long long>();
	}
	if( filter != NULL ){
		text = filter->readText(in_file, text_length, lowcase_runs);
		text_length = filter->filterNewLines(text, text_length, nl_pos);
	}
	if( text == NULL ){
		cerr<<"Compressor::realCompress - Error en carga de texto\n";
		if(lowcase_runs != NULL){
			delete lowcase_runs;
		}
		if(nl_pos != NULL){
			delete nl_pos;
		}
		return false;
	}
	
	cout<<"Compressor::realCompress - Texto de "<<text_length<<" chars\n";
//	cout<<"Compressor::realCompress - Texto: \""<<text<<"\" ("<<text_length<<")\n";
	if( use_metadata ){
		cout<<"Compressor::realCompress - lowcase_runs: "<<lowcase_runs->size()<<", nl_pos: "<<nl_pos->size()<<"\n";
	}
	
	unsigned int n_blocks = (unsigned int)(text_length / block_size);
	if( (unsigned long long)n_blocks * block_size < text_length ){
		++n_blocks;
	}
	
	vector< pair<char *, unsigned int> > lista_textos;
	
	unsigned long long stored_text = 0;
	unsigned int largo_local = 0;
	for(unsigned int block = 0; block < n_blocks; ++block){
		if( text_length - stored_text < (unsigned long long)block_size ){
			largo_local = (unsigned int)(text_length - stored_text);
		}
		else{
			largo_local = block_size;
		}
		lista_textos.push_back(pair<char *, unsigned int>(text + stored_text, largo_local));
		stored_text += largo_local;
	}
	
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
		datos_threads[i].shared_pos = &shared_pos;
		
		datos_threads[i].shared_mutex = &shared_mutex;
		datos_threads[i].lista_textos = &lista_textos;
		datos_threads[i].vector_bytes_headers = &vector_bytes_headers;
		datos_threads[i].vector_bytes_data = &vector_bytes_data;
		datos_threads[i].vector_thread_block = &vector_thread_block;
		
	}
	
	cout<<"Compressor::realCompress - Iniciando Threads de Compresion\n";
	NanoTimer timer;
	for(unsigned int i = 0; i < n_threads; ++i){
		compress_threads.push_back( std::thread(thread_compress, &(datos_threads[i]) ) );
	}
	for(unsigned int i = 0; i < n_threads; ++i){
		compress_threads[i].join();
	}
	lista_textos.clear();
	delete [] text;
	cout<<"Compressor::realCompress - Threads terminados en "<<timer.getMilisec()<<" ms, iniciando Merge\n";
	
	//Merge en el archivo maestro
	
	//Total de bytes leidos hasta el momento de los archivos de headers/data de cada thread
	unsigned int bytes_headers_thread[n_threads];
	unsigned int bytes_data_thread[n_threads];
	memset(bytes_headers_thread, 0, n_threads * sizeof(int));
	memset(bytes_data_thread, 0, n_threads * sizeof(int));
	
	cout<<"Compressor::realCompress - Preparando nuevo header\n";
	//Escritura de headers en el master (headers es del tipo de decoder->headers)
	BlockHeaders *headers = decoder->getNewHeaders(text_length, block_size, new Metadata(META_SAVE_VBYTE, lowcase_runs, nl_pos) );
	
	//Buffer para la copia
	unsigned int buffer_size = 1024*1024;
	char *buff_lectura = new char[buffer_size];
	fstream lector;
	
	cout<<"Compressor::realCompress - Cargando Headers\n";
	for(unsigned int block = 0; block < n_blocks; ++block){
		unsigned int thread_id = vector_thread_block[block];
		
		//Cargar el header particular
		//Los bytes son para asegurar la lectura del header correcto
		unsigned int bytes_headers = vector_bytes_headers[block];
		//Lectura de archivo del thread
		lector.open(datos_threads[thread_id].file_headers, fstream::binary | fstream::in);
		if( (! lector.good()) || (! lector.is_open()) ){
			cerr<<"Compressor::realCompress - Error en lectura de \""<<datos_threads[thread_id].file_headers<<"\"\n";
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
//	cout<<"Compressor::realCompress - Escribiendo Headers en \""<<master_file<<"\"\n";
//	headers->save(&escritor);
	BlockHeadersFactory::save(headers, &escritor);
	delete headers;
//	cout<<"Compressor::realCompress - Agregando Datos\n";
	for(unsigned int block = 0; block < n_blocks; ++block){
		unsigned int thread_id = vector_thread_block[block];
		
//		cout<<"Compressor::realCompress - Leyendo Bloque "<<block<<" / "<<n_blocks<<"\n";
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
			cerr<<"Compressor::realCompress - Error en lectura de \""<<datos_threads[thread_id].file_data<<"\"\n";
			break;
		}
//		cout<<"Compressor::realCompress - seekg...\n";
		lector.seekg(bytes_data_thread[thread_id], lector.beg);
//		cout<<"Compressor::realCompress - read...\n";
		lector.read(buff_lectura, bytes_data);
//		cout<<"Compressor::realCompress - close...\n";
		lector.close();
		//escritura de bytes
//		cout<<"Compressor::realCompress - Escribiendo\n";
		escritor.write(buff_lectura, bytes_data);
		bytes_data_thread[thread_id] += bytes_data;
//		cout<<"Compressor::realCompress - Ok\n";
		
	}
	
	delete [] buff_lectura;
//	cout<<"Compressor::realCompress - close...\n";
	escritor.close();
	
//	cout<<"Compressor::realCompress - Borrando Archivos\n";
	//Eliminacion de archivos de threads
	for(unsigned int i = 0; i < n_threads; ++i){
		remove(datos_threads[i].file_headers);
		remove(datos_threads[i].file_data);
	}
	cout<<"Compressor::realCompress - Fin\n";
	return true;
	
}









