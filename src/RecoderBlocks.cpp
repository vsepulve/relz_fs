#include "RecoderBlocks.h"

RecoderBlocks::RecoderBlocks(ReferenceIndex *_referencia){
	referencia = _referencia;
}

RecoderBlocks::~RecoderBlocks(){
//	if(buff_pos != NULL){
//		delete [] buff_pos;
//		buff_pos = NULL;
//	}
//	if(buff_len != NULL){
//		delete [] buff_len;
//		buff_len = NULL;
//	}
}

//void thread_recompress_blocks(unsigned int id, vector< pair<char*, unsigned int> > *lista_textos, unsigned int n_blocks, mutex *shared_mutex, unsigned int *shared_pos, vector<unsigned int> *vector_bytes_pos, vector<unsigned int> *vector_bytes_len, vector<unsigned int> *vector_n_factores, vector<unsigned int> *vector_thread_block, ReferenceIndex *referencia) {

void thread_recompress_blocks(RecoderBlocks::ThreadCompressData *data) {
	
	//Datos compartidos (punteros)
	ReferenceIndex *referencia = data->referencia;
	unsigned int *shared_pos = data->shared_pos;
	mutex *shared_mutex = data->shared_mutex;
//	vector< pair<char*, unsigned int> > *lista_textos = data->lista_textos;
	vector<unsigned int> *vector_bytes_pos = data->vector_bytes_pos;
	vector<unsigned int> *vector_bytes_len = data->vector_bytes_len;
	vector<unsigned int> *vector_n_factores = data->vector_n_factores;
	vector<unsigned int> *vector_thread_block = data->vector_thread_block;
	
//	shared_mutex->lock();
//	cout<<"RecoderBlocks::Thread ["<<data->id<<"] - Inicio\n";
//	shared_mutex->unlock();
	
	BitsUtils utils;
	
	unsigned int *buff_pos = new unsigned int[data->block_size + 1];
	unsigned int *buff_len = new unsigned int[data->block_size + 1];

	char archivo_len_thread[512];
	sprintf(archivo_len_thread, "len_file.tmp.%d", data->id);
	
	char archivo_pos_thread[512];
	sprintf(archivo_pos_thread, "pos_file.tmp.%d", data->id);
	
	DecoderBlocksBytes decoder(data->bytes_lectura, referencia->getText());
	
	LengthsCoderBlocks lengths_coder(archivo_len_thread);
	PositionsCoderBlocks positions_coder(archivo_pos_thread);
	
	unsigned int block = 0;
	unsigned int blocks_procesados = 0;
	
	char *text = new char[data->block_size + 1];
	unsigned int largo_local = 0;
	unsigned int compressed_text = 0;
	unsigned int n_factores = 0;
	unsigned int max_pos = 0;
	
	unsigned int pos_prefijo, largo_prefijo;
	
	while(true){
		shared_mutex->lock();
		block = (*shared_pos)++;
//		cout<<"Thread ["<<data->id<<"] - Procesando block "<<block<<" (salir? "<<(block >= data->n_blocks)<<")\n";
		shared_mutex->unlock();
		if(block >= data->n_blocks){
			break;
		}
		
		compressed_text = 0;
//		text = lista_textos->at(block).first;
//		largo_local = lista_textos->at(block).second;

		//Descomprimir el texto localmente
		largo_local = decoder.decodeBlock(block, text);
		
		n_factores = 0;
		max_pos = 0;
		while(largo_local > 0){
			referencia->find(text + compressed_text, largo_local, pos_prefijo, largo_prefijo);
			
			if(largo_prefijo == 0){
				shared_mutex->lock();
				cerr<<"RecoderBlocks::Thread ["<<data->id<<"] - Error - Prefijo de largo 0, saliendo\n";
				shared_mutex->unlock();
				return;
			}
			
			largo_local -= largo_prefijo;
			compressed_text += largo_prefijo;
			if(pos_prefijo > max_pos){
				max_pos = pos_prefijo;
			}
			
//			shared_mutex->lock();
//			cout<<"Thread ["<<data->id<<"] - ("<<pos_prefijo<<", "<<largo_prefijo<<")\n";
//			shared_mutex->unlock();

			buff_pos[n_factores] = pos_prefijo;
			buff_len[n_factores] = largo_prefijo;
			++n_factores;
			
		}//while... procesar factores
		
		//Esto indica que thread proceso cada bloque
		//Con eso, y la informacion posicional, se pueden reconstruir los archivos maestros
		(*vector_thread_block)[block] = data->id;
		
		(*vector_n_factores)[block] = n_factores;
		
		(*vector_bytes_pos)[block] = positions_coder.encodeBlockMaxBits(buff_pos, n_factores, utils.n_bits(max_pos));
		
		(*vector_bytes_len)[block] = lengths_coder.encodeBlockGolomb(buff_len, n_factores);
		++blocks_procesados;
		
	}//while... procesar bloques
	
	delete [] buff_len;
	delete [] buff_pos;
	delete [] text;
	
	positions_coder.close();
	lengths_coder.close();
	
	
	shared_mutex->lock();
	cout<<"RecoderBlocks::Thread ["<<data->id<<"] - Fin ("<<blocks_procesados<<" bloques procesados)\n";
	shared_mutex->unlock();
	
	return;
	
}

//void RecoderBlocks::compress(const char *archivo_entrada, const char *archivo_salida, unsigned int n_threads, unsigned int block_size){
void RecoderBlocks::compress(char *bytes, unsigned int total_bytes, const char *archivo_salida, unsigned int n_threads){

	cout<<"RecoderBlocks::compress - Inicio (Archivo \""<<archivo_salida<<"\")\n";
	NanoTimer timer;
	
//	unsigned int text_size = 0;
	
	//Inicio de datos desde bytes
	
	fstream lector;
	BytesReader lector_bytes(bytes);
	
	unsigned int byte_ini_headers_lectura = 0;
	lector_bytes.read((char*)(&byte_ini_headers_lectura), sizeof(int));
	
	cout<<"RecoderBlocks::compress - Posiciones de inicio ("<<byte_ini_headers_lectura<<")\n";
	
	BlockHeaders headers_lectura;
	headers_lectura.openBytes(bytes, byte_ini_headers_lectura);
	
	unsigned int text_size = headers_lectura.getTextSize();
	unsigned int block_size = headers_lectura.getBlockSize();
	
	//Versiones de posiciones y largos?
	BlockHeaders *headers = new BlockHeaders(text_size, block_size);
	
	//Notar que considero SOLO LOS BLOQUES REALES (omito el bloque de marca final)
	unsigned int n_blocks = headers_lectura.getNumBlocks() - 1;
//	unsigned int n_blocks = text_size / block_size;
//	if(n_blocks * block_size < text_size){
//		++n_blocks;
//	}
	
	cout<<"RecoderBlocks::compress - Preparando "<<n_blocks<<" bloques ("<<text_size<<" chars en bloques de "<<block_size<<")\n";
	
	//aqui habria que crear una lista de bloques de texto (char* text, int size) para procesar
	//luego pueden lanzarse los threads para procesar
//	vector< pair<char *, unsigned int> > lista_textos;
	
//	unsigned int stored_text = 0;
//	unsigned int largo_local = 0;
//	for(unsigned int block = 0; block < n_blocks; ++block){
//		if(text_size - stored_text < block_size){
//			largo_local = text_size - stored_text;
//		}
//		else{
//			largo_local = block_size;
//		}
//		lista_textos.push_back(pair<char *, unsigned int>(text + stored_text, largo_local));
//		stored_text += largo_local;
//		
//	}
	
	vector<std::thread> sort_threads;
	unsigned int shared_pos = 0;
	mutex shared_mutex;
	//Vectores para que los threads escriban los datos de cada bloque
	//Esos datos se usan despues para generar los headers de bloques
	vector<unsigned int> vector_bytes_pos(n_blocks+1);
	vector<unsigned int> vector_bytes_len(n_blocks+1);
	vector<unsigned int> vector_n_factores(n_blocks+1);
	vector<unsigned int> vector_thread_block(n_blocks+1);
	
	ThreadCompressData datos_threads[n_threads];
	for(unsigned int i = 0; i < n_threads; ++i){
	
		datos_threads[i].id = i;
		datos_threads[i].n_blocks = n_blocks;
		datos_threads[i].block_size = block_size;
		
		datos_threads[i].referencia = referencia;
		datos_threads[i].shared_pos = &shared_pos;
		
		datos_threads[i].shared_mutex = &shared_mutex;
//		datos_threads[i].lista_textos = &lista_textos;
		datos_threads[i].bytes_lectura = bytes;
		datos_threads[i].vector_bytes_pos = &vector_bytes_pos;
		datos_threads[i].vector_bytes_len = &vector_bytes_len;
		datos_threads[i].vector_n_factores = &vector_n_factores;
		datos_threads[i].vector_thread_block = &vector_thread_block;
		
	}
	
	cout<<"RecoderBlocks::compress - Iniciando Threads de Compresion\n";
	timer.reset();
	for(unsigned int i = 0; i < n_threads; ++i){
		sort_threads.push_back( std::thread(thread_recompress_blocks, &(datos_threads[i]) ) );
	}
	for(unsigned int i = 0; i < n_threads; ++i){
		sort_threads[i].join();
	}
//	lista_textos.clear();
	cout<<"RecoderBlocks::compress - Compresion terminada en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"RecoderBlocks::compress - Iniciando Merge de Archivo Maestro\n";
	timer.reset();
	
	//Notar que los primeros bytes son marcas de tamaño fijo
	//Quizas se pueda agregar info de version, referencia, etc
	//Por ahora mantengo 3 enteros (12 bytes) para los inicios de cada parte del archivo
	//Los otros dos son sobreescritos al final
	unsigned int byte_ini_headers = 12;
	unsigned int byte_ini_pos = 0;
	unsigned int byte_ini_len = 0;
	fstream escritor(archivo_salida, fstream::trunc | fstream::binary | fstream::out);
	escritor.write((char*)(&byte_ini_headers), sizeof(int));
	escritor.write((char*)(&byte_ini_pos), sizeof(int));
	escritor.write((char*)(&byte_ini_len), sizeof(int));
	escritor.close();
	
	unsigned int bytes_total_pos = 0;
	unsigned int bytes_total_len = 0;
	unsigned int bytes_pos = 0;
	unsigned int bytes_len = 0;
	unsigned int bytes_leidos_thread_pos[n_threads];
	unsigned int bytes_leidos_thread_len[n_threads];
	memset(bytes_leidos_thread_pos, 0, n_threads * sizeof(int));
	memset(bytes_leidos_thread_len, 0, n_threads * sizeof(int));
	
	//Preparacion de Headers (notar que los bytes de un bloque van en el bloque siguiente)
	//max_bytes es para el tamaño del buffer de lectura para la construccion de archivos maestros
	unsigned int max_bytes = 0;
	headers->addBlock(vector_n_factores[0], 0, 0);
	
	for(unsigned int block = 0; block < n_blocks; ++block){
		
		//tomo el thread
		unsigned int thread_id = vector_thread_block[block];
		//calculo los bytes del bloque actual en los archivos del thread
		bytes_pos = vector_bytes_pos[block] - bytes_leidos_thread_pos[thread_id];
		bytes_len = vector_bytes_len[block] - bytes_leidos_thread_len[thread_id];
		//sumo los bytes a los ya leidos de los archivos del thread
		bytes_leidos_thread_pos[thread_id] += bytes_pos;
		bytes_leidos_thread_len[thread_id] += bytes_len;
		//tambien sumo los bytes a los ya leidos en el archivo maestro
		bytes_total_pos += bytes_pos;
		bytes_total_len += bytes_len;
		//max_bytes es para el buffer que uso despues
		if( bytes_pos > max_bytes ){
			max_bytes = bytes_pos;
		}
		if( bytes_len > max_bytes ){
			max_bytes = bytes_len;
		}
		
		headers->addBlock(vector_n_factores[block+1], bytes_total_pos, bytes_total_len);
	}
	
//	cout<<"RecoderBlocks::compress - Guardando Headers\n";
	byte_ini_pos = headers->save(archivo_salida, true);
	
	char *buff_lectura = new char[max_bytes + 1];
	char *nombre_archivo = new char[1024];
	
//	cout<<"RecoderBlocks::compress - Guardando Posiciones\n";
	memset(bytes_leidos_thread_pos, 0, n_threads * sizeof(int));
	
	escritor.open(archivo_salida, fstream::app | fstream::binary | fstream::out);
	for(unsigned int block = 0; block < n_blocks; ++block){
		unsigned int thread_id = vector_thread_block[block];
		//copiar desde archivo apropiado, posicion actual y hasta total de bytes
		//Notar que cada thread ha escrito bloques CRECIENTES en sus archivos
		//De este modo, siempre se puede restar la pos del bloque actual con la lectura actual de ese thread
		bytes_pos = vector_bytes_pos[block] - bytes_leidos_thread_pos[thread_id];
		
		//lectura de bytes
		sprintf(nombre_archivo, "pos_file.tmp.%d", thread_id);
//		cout<<"block["<<block<<"]: Copiando "<<bytes_pos<<" bytes de "<<nombre_archivo<<" a "<<archivo_salida_pos<<"\n";
		lector.open(nombre_archivo, fstream::binary | fstream::in);
		lector.seekg(bytes_leidos_thread_pos[thread_id], lector.beg);
		lector.read(buff_lectura, bytes_pos);
		lector.close();
		
		//escritura de bytes
		escritor.write(buff_lectura, bytes_pos);
		 
		bytes_leidos_thread_pos[thread_id] += bytes_pos;
		
	}
	byte_ini_len = escritor.tellp();
	
//	cout<<"RecoderBlocks::compress - Guardando Largos\n";
	
	memset(bytes_leidos_thread_len, 0, n_threads * sizeof(int));
	
	for(unsigned int block = 0; block < n_blocks; ++block){
		unsigned int thread_id = vector_thread_block[block];
		//copiar desde archivo apropiado, posicion actual y hasta total de bytes
		bytes_len = vector_bytes_len[block] - bytes_leidos_thread_len[thread_id];
		
		
		//lectura de bytes
		sprintf(nombre_archivo, "len_file.tmp.%d", thread_id);
//		cout<<"block["<<block<<"]: Copiando "<<bytes_len<<" bytes de "<<nombre_archivo<<" a "<<archivo_salida_len<<"\n";
		lector.open(nombre_archivo, fstream::binary | fstream::in);
		lector.seekg(bytes_leidos_thread_len[thread_id], lector.beg);
		lector.read(buff_lectura, bytes_len);
		lector.close();
		
		//escritura de bytes
		escritor.write(buff_lectura, bytes_len);
		
		bytes_leidos_thread_len[thread_id] += bytes_len;
		
	}
	escritor.close();
	
	//Ajuste final de posiciones
	//Notar que app impide el seekp / write, por lo que hay que reabrir con (in | out)
	escritor.open(archivo_salida, fstream::binary | fstream::in | fstream::out);
	escritor.seekp(0);
	escritor.write((char*)(&byte_ini_headers), sizeof(int));
	escritor.write((char*)(&byte_ini_pos), sizeof(int));
	escritor.write((char*)(&byte_ini_len), sizeof(int));
	escritor.close();
	
	cout<<"RecoderBlocks::compress - Merge Terminado en "<<timer.getMilisec()<<" ms\n";
	
	for(unsigned int i = 0; i < n_threads; ++i){
		sprintf(nombre_archivo, "pos_file.tmp.%d", i);
		remove(nombre_archivo);
		sprintf(nombre_archivo, "len_file.tmp.%d", i);
		remove(nombre_archivo);
	}
	
//	delete [] text;
	
	
	
	
	
}











