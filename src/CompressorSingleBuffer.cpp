#include "CompressorSingleBuffer.h"

CompressorSingleBuffer::CompressorSingleBuffer()
: Compressor()
{
	cout<<"CompressorSingleBuffer - Inicio\n";
	cur_block = 0xffffffff;
	cur_block_size = 0;
	buffer_size = 0;
	buffer = NULL;
	adjust_buffer = NULL;
	cout<<"CompressorSingleBuffer - Fin\n";
}

CompressorSingleBuffer::CompressorSingleBuffer(const char *_master_file, CoderBlocks *_coder, DecoderBlocks *_decoder, TextFilter *_filter)
: Compressor(_master_file, _coder, _decoder, _filter)
{
	cout<<"CompressorSingleBuffer - Inicio\n";
	cur_block = 0xffffffff;
	cur_block_size = 0;
	buffer_size = 0;
	buffer = NULL;
	adjust_buffer = NULL;
	cout<<"CompressorSingleBuffer - Fin\n";
}

CompressorSingleBuffer::~CompressorSingleBuffer(){
	if(buffer != NULL){
		delete [] buffer;
		buffer = NULL;
		buffer_size = 0;
	}
	if(adjust_buffer != NULL){
		delete [] adjust_buffer;
		adjust_buffer = NULL;
		buffer_size = 0;
	}
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

void CompressorSingleBuffer::prepareBuffer(unsigned int new_size){
	if( new_size > buffer_size ){
		if(buffer != NULL){
			delete [] buffer;
			buffer = NULL;
		}
		if(adjust_buffer != NULL){
			delete [] adjust_buffer;
			adjust_buffer = NULL;
		}
		buffer_size = new_size;
		buffer = new char[buffer_size];
		adjust_buffer = new char[buffer_size];
	}
}

unsigned int CompressorSingleBuffer::read(unsigned long long pos_ini, unsigned int length, char *out_buff){
	//Verificacion de seguridad
	if( decoder == NULL || master_file == NULL || strlen(master_file) < 1 ){
		cerr<<"CompressorSingleBuffer::read - Datos incorrectos ("<<(decoder == NULL)<<" || "<<(master_file == NULL)<<" || "<<(strlen(master_file) < 1)<<")\n";
		return 0;
	}
	if( length == 0 ){
		out_buff[0] = 0;
		return 0;
	}
	//Calcular bloques involucrados
	//Iterar por cada bloque
	//	Descomprimir el bloque
	//	Calcular limites del bloque
	//	memcpy del buffer_interno al buff salida
	//Retornar total copiado
	
	//Lock desde aqui hasta el final del metodo
	lock_guard<mutex> lock(mutex_interno);
	
	bool debug = false;
//	if( pos_ini > 4285000000 && pos_ini < 4300000000 ){
//		debug = true;
//	}
	
	if(debug) cout<<"CompressorSingleBuffer::read - Inicio ("<<pos_ini<<", "<<length<<", total: "<<decoder->getTextSize()<<")\n";
	
	//Revision del buffer
	//Aseguro que tena suficiente para el bloque y tambien para el texto pedido
	//Eso es porque el mismo buffer se usara para reescribir en caso de ajustes
	prepareBuffer(decoder->getBlockSize() + 1);
	prepareBuffer(length + 1);
	
	//Ajuste a posiciones relativas
	unsigned int nl_izq = 0;
	unsigned int nl_med = 0;
	if( decoder->getHeaders() != NULL && adjust_text){
		nl_izq = decoder->getHeaders()->countNewLines(pos_ini);
		nl_med = decoder->getHeaders()->countNewLines(pos_ini + length);
		if(debug) cout<<"CompressorSingleBuffer::read - nl_izq: "<<nl_izq<<", nl_med: "<<nl_med<<" - "<<nl_izq<<"\n";
		nl_med -= nl_izq;
		if( (nl_izq > pos_ini) || (nl_med > length) ){
			cerr<<"CompressorSingleBuffer::read - Error al ajustar posiciones ("<<nl_izq<<" de "<<pos_ini<<", "<<nl_med<<" de "<<length<<")\n";
			nl_izq = 0;
			nl_med = 0;
		}
		else{
			pos_ini -= nl_izq;
			length -= nl_med;
			if(debug) cout<<"CompressorSingleBuffer::read - pos/len relativas: ("<<pos_ini<<", "<<length<<")\n";
		}
	}
	
	bool procesar = true;
	
	if( pos_ini >= decoder->getTextSize() || length == 0 ){
		if(debug) cout<<"CompressorSingleBuffer::read - fuera de rango, omitiendo proceso\n";
		procesar = false;
	}
	
	unsigned int block = (unsigned int)( pos_ini / decoder->getBlockSize() );
	unsigned int block_fin = (unsigned int)( (pos_ini + length) / decoder->getBlockSize() );
	if(block_fin > decoder->getNumBlocks()-1){
		block_fin = decoder->getNumBlocks()-1;
	}
	
	if(debug) cout<<"CompressorSingleBuffer::read - Procesando bloques ["<<block<<", "<<block_fin<<"] de "<<decoder->getNumBlocks()<<" (buffer_size: "<<buffer_size<<")\n";
	
	//total de caracteres copiados
	unsigned int copied_chars = 0;
	//ini de copia en el bloque actual. Normalmente es 0, solo puede ser diferente en el primer bloque
	unsigned int ini_copy = 0;
	//chars a copiar del bloque actual (el tamaño real del bloque, excepto al final)
	unsigned int copy_length = 0;
	//Posicion de inicio del bloque actual en el texto comprimido
	unsigned long long cur_block_ini = 0;
	
	for( ; (block <= block_fin) && procesar ; ++block ){
		if(block != cur_block){
			if(debug) cout<<"CompressorSingleBuffer::read - decoder->decodeBlock "<<block<<"\n";
			cur_block_size = decoder->decodeBlock(block, buffer);
			cur_block = block;
			if(debug) cout<<"CompressorSingleBuffer::read - buff: \""<<( string(buffer, (cur_block_size<10)?cur_block_size:10 ) )<<"\"\n";
		}
		cur_block_ini = (unsigned long long)block * decoder->getBlockSize();
		
		if(debug) cout<<"CompressorSingleBuffer::read - cur_block_ini: "<<cur_block_ini<<", cur_block_size: "<<cur_block_size<<"\n";
		
		ini_copy = 0;
		if(cur_block_ini < pos_ini){
			if( pos_ini - cur_block_ini > 0xffffffff ){
				cerr<<"CompressorSingleBuffer::read - Error, valor > 32 bits\n";
				break;
			}
			ini_copy = (unsigned int)(pos_ini - cur_block_ini);
		}
		if(debug) cout<<"CompressorSingleBuffer::read - ini_copy: "<<ini_copy<<"\n";
		
		copy_length = cur_block_size - ini_copy;
		if( copy_length > length ){
			copy_length = length;
		}
		
		if(debug) cout<<"CompressorSingleBuffer::read - memcpy (out_buff["<<copied_chars<<"], buffer["<<ini_copy<<"], "<<copy_length<<")\n";
		memcpy( out_buff + copied_chars, buffer + ini_copy, copy_length );
		
		length -= copy_length;
		copied_chars += copy_length;
		out_buff[copied_chars] = 0;
//		cout<<"CompressorSingleBuffer::read - out_buff: \""<<out_buff<<"\"\n";
		
		if( length < 1 ){
			break;
		}
		
	}//for... cada bloque
	
	out_buff[copied_chars] = 0;
	
	if(debug) cout<<"CompressorSingleBuffer::read - copied_chars: "<<copied_chars<<" (antes de ajustes: \""<<((strlen(out_buff)>10)?string(out_buff, 10):out_buff)<<"...\")\n";
	
	if( decoder->getHeaders() != NULL && adjust_text){
		//Aqui habria que agregar '\n' si fueron filtradas.
		//Para eso se requiere un flag de new line que indique como proceder (guardadas en texto vs filtradas a metadatos)
		//Ademas, la agregacion debe realizarse ANTES de adjustCase para ajustar a las posiciones absolutas
		//En ese caso, es necesario haber ajustado las posiciones absolutas a relativas ANTES de descomprimir el texto
		//Una opcion para simplificar esto es encapsular el read relativo y llamarlo desde uno absoluto que haga todos los ajustes
		
		pos_ini += nl_izq;
		decoder->getHeaders()->adjustNewLines(out_buff, pos_ini, copied_chars, nl_izq, nl_med, adjust_buffer);
		copied_chars += nl_med;
		
		decoder->getHeaders()->adjustCase(out_buff, pos_ini, copied_chars);
	}
	
	if(debug) cout<<"CompressorSingleBuffer::read - Fin (copied_chars: "<<copied_chars<<", strlen: "<<strlen(out_buff)<<")\n";
	
	return copied_chars;
}

bool CompressorSingleBuffer::decompress(const char *out_file, unsigned int line_size){
	cout<<"CompressorSingleBuffer::decompress - Inicio\n";
	//Verificacion de seguridad
	if( decoder == NULL || out_file == NULL || strlen(out_file) < 1 || master_file == NULL || strlen(master_file) < 1){
		cerr<<"CompressorSingleBuffer::decompress - Datos incorrectos\n";
		return false;
	}
	//El lock esta en el read, no es necesario aqui
	//Descompresion
	NanoTimer timer;
	fstream escritor(out_file, fstream::trunc | fstream::out);
	if(! escritor.good() ){
		cerr<<"CompressorSingleBuffer::decompress - Problemas al abrir archivo\n";
		return false;
	}
//	cout<<"CompressorSingleBuffer::decompress - Preparando buffer de linea\n";
	char *line = new char[line_size + 1];
	unsigned int real_size = 0;
	unsigned long long ini = 0;
	cout<<"CompressorSingleBuffer::decompress - Iniciando read por linea\n";
	ini += read(ini, line_size, line);
//	cout<<"CompressorSingleBuffer::decompress - Linea ("<<strlen(line)<<" chars, \""<<line<<"\")\n";
//	cout<<"CompressorSingleBuffer::decompress - Linea ("<<strlen(line)<<" chars)\n";
	while( (real_size = strlen(line)) > 0 ){
	
		//Agregación artificial del newline
//		sprintf(line + real_size, "\n");
//		escritor.write(line, real_size + 1);
		
		//Escritura directa del read (esto es cuando se usan metadatos)
		escritor.write(line, real_size );
		
		ini += read(ini, line_size, line);
//		cout<<"CompressorSingleBuffer::decompress - Linea ("<<strlen(line)<<" chars, \""<<line<<"\")\n";
//		cout<<"CompressorSingleBuffer::decompress - Linea ("<<strlen(line)<<" chars)\n";
	}
	cout<<"CompressorSingleBuffer::decompress - Escritura terminada, cerrando y liberando buffer\n";
	escritor.flush();
	escritor.close();
	delete [] line;
	cout<<"CompressorSingleBuffer::decompress - Fin ("<<ini<<" chars en "<<timer.getMilisec()<<" ms)\n";
	return true;
}

unsigned int CompressorSingleBuffer::write(const char *original_text, unsigned int original_length, unsigned long long original_pos_ini){

	bool implementacion_terminada = true;
	if(! implementacion_terminada){
		cerr<<"CompressorSingleBuffer::write - No Implementado\n";
		return 0;
	}
	
	//Verificacion de seguridad
	if(coder == NULL || decoder == NULL || decoder->getHeaders() == NULL || master_file == NULL || strlen(master_file) < 1){
		cerr<<"CompressorSingleBuffer::write - Datos incorrectos\n";
		return 0;
	}
	//Calcular bloques afectados
	//Iterar por cada bloque afectado
	//	Descomprimir bloque en buffer
	//	Modificar el bloque
	//	Recomprimir el bloque en otro archivo
	//Ajustar el archivo maestro con bloques recomprimidos
	
	//Lock desde aqui hasta el final del metodo
	lock_guard<mutex> lock(mutex_interno);
	
	if( original_pos_ini > decoder->getTextSize() || original_length == 0 ){
		cout<<"CompressorSingleBuffer::write - Escritura invalida (original_pos_ini: "<<original_pos_ini<<" de "<<decoder->getTextSize()<<", original_length: "<<original_length<<")\n";
		return 0;
	}
	
	//Prepara archivos temporales (idealmente basados en master_file, que esta lockeado)
	const char *path_headers = "/cebib_yeast_real/headers.tmp";
	const char *path_data = "/cebib_yeast_real/data.tmp";
	const char *path_merge = "/cebib_yeast_real/merge.tmp";
	
	cout<<"CompressorSingleBuffer::write - Inicio ("<<original_length<<" chars en "<<original_pos_ini<<" usando "<<path_headers<<", "<<path_data<<", "<<path_merge<<")\n";
	
	//Revision del buffer
	prepareBuffer(decoder->getBlockSize() + 1);
	
	// Aqui habria que filtrar el texto de entrada para extraer metadatos
	unsigned int length = original_length;
	unsigned long long pos_ini = original_pos_ini;
	char *text = new char[length + 1];
	text[0] = 0;
	if( adjust_text ){
		// Usar un metodo de metadatos que reciba el buffer, lo revise y guarde todo lo necesario, y lo deje filtrado
		decoder->getHeaders()->filterNewText(original_text, original_length, original_pos_ini, text, length, pos_ini);
		cout<<"CompressorSingleBuffer::write - Texto filtrado (length: "<<length<<", pos_ini: "<<pos_ini<<")\n";
		
	}
	
	//Notar que en este caso es valido un block_fin > ultimo bloque, debe verificarse en el ciclo
	unsigned int block_ini = (unsigned int)( pos_ini / decoder->getBlockSize() );
	unsigned int block_fin = (unsigned int)( (pos_ini + length) / decoder->getBlockSize() );
	unsigned int block = block_ini;
	
	//total de caracteres copiados
//	unsigned int copied_chars = 0;
	//ini de copia en el bloque actual. Normalmente es 0, solo puede ser diferente en el primer bloque
	unsigned int ini_copy = 0;
	//chars a copiar del bloque actual (el tamaño real del bloque, excepto al final)
	unsigned int copy_length = 0;
	//Posicion de inicio del bloque actual en el texto comprimido
	unsigned long long cur_block_ini = 0;
	//Tamaño efectivo del bloque (block size salvo por el ultimo)
	unsigned int real_size = 0;
	
	// Variables para la compresion y merge
	vector<unsigned int> vector_bytes_headers;
	vector<unsigned int> vector_bytes_data;
	unsigned int bytes_headers = 0;
	unsigned int bytes_data = 0;
	unsigned int full_buffer_size = coder->codingBufferSize( decoder->getBlockSize() );
	// Defino un minimo al buffer pues lo usare luego para las copias (read/write)
	if( full_buffer_size < 1024*128 ){
		full_buffer_size = 1024*128;
	}
	char *full_buffer = new char[ full_buffer_size ];
	// Tomo headers y deshago la preparacion desde block en adelante (=> anulo la acumulacion)
	// Notar que luego sera necesario un prepare solo desde block (se puede redefinir con block = 0 por defecto)
	// Tambien notar que ambos procesos son SIEMPRE desde un cierto block en adelante
	BlockHeaders *headers = decoder->getHeaders();
	cout<<"CompressorSingleBuffer::write - n_blocks original: "<<headers->getNumBlocks()<<"\n";
	// Muevo el unprepare para justo antes de cargar los headers pues lo necesito usable en la compresion
//	headers->unprepare(block);
	// Para la primera copia se necesita la pos de inicio y termino en bytes del rango de bloques
	// Esto puede preguntarsele a headers con algun metodo especial que calcule el rango
	unsigned int first_copy_start = headers->getDataPosition();
	unsigned int first_copy_end = headers->getBlockPosition(block_fin);
	unsigned int third_copy_start = 0;
	unsigned int third_copy_end = 0;
	if( block_fin < headers->getNumBlocks() ){
		third_copy_start = headers->getBlockPosition(block_fin + 1);
		// La llamada que sigue funciona, pero quizas sea mejor implementarla de forma mas clara
		third_copy_end = headers->getBlockPosition(headers->getNumBlocks() + 1);
	}
	fstream file_headers(path_headers, fstream::trunc | fstream::binary | fstream::out);
	fstream file_data(path_data, fstream::trunc | fstream::binary | fstream::out);
	unsigned int total_bytes_data = 0;
	unsigned int total_copied_chars = 0;
	// Ajusto de inmediato el largo del texto
	// Notar que esto PODRIA hacerse mejor con alguna verificacion posterior
	headers->increaseTextSize(pos_ini + length);
	
	for( ; block <= block_fin; ++block ){
	
		// Si el bloque es mayor al ultimo, no se puede decodificar
		// En ese caso simplemente se necesita el buffer (decodeBlock deberia funcionar en cualquier caso)
		
		real_size = decoder->decodeBlock(block, buffer);
		cout<<"CompressorSingleBuffer::write - buffer: \""<<buffer<<"\"\n";
		
		cur_block_ini = (unsigned long long)block * decoder->getBlockSize();
		
		ini_copy = 0;
		if(cur_block_ini < pos_ini){
			if( pos_ini - cur_block_ini > 0xffffffff ){
				cerr<<"CompressorSingleBuffer::write - Error, valor > 32 bits\n";
				break;
			}
			ini_copy = (unsigned int)(pos_ini - cur_block_ini);
		}
		
		copy_length = real_size;
		if( real_size == decoder->getBlockSize() ){
			//bloque completo
			if( real_size > length ){
				copy_length = length;
			}
		}
		else{
			//bloque incompleto o vacio
			//el unico caso especial es si, en un bloque vacio, real_size es mayor a length
			//En ese caso se trata igual al caso normal
			if( real_size > length ){
				copy_length = length;
			}
			else if( length < decoder->getBlockSize() ){
				copy_length = real_size = length;
			}
			else{
				copy_length = real_size = decoder->getBlockSize();
			}
		}
		
		// Notar que text y length se mueven para copiar siempre desde el inicio
		cout<<"CompressorSingleBuffer::write - memcpy( buffer + "<<ini_copy<<", text + "<<total_copied_chars<<", "<<copy_length<<" )\n";
		memcpy( buffer + ini_copy, text + total_copied_chars, copy_length );
		
		cout<<"buffer: \""<<(string(buffer, ini_copy+copy_length))<<"\"\n";
		
		//Comprimir Bloque (con append en archivo temporal)
		// Notar que file_x son fstreams abiertos y validos
		// Tambien se requiere un buffer externo pedido previamente (full_buffer, no se si del metodo o de la clase)
		// por ultimo, hay que guardar bytes_x
		cout<<"CompressorSingleBuffer::write - coder->codeBlock...\n";
		coder->codeBlock(buffer, ini_copy+copy_length, &file_headers, &file_data, bytes_headers, bytes_data, full_buffer);
		
		cout<<"CompressorSingleBuffer::write - bytes_headers: "<<bytes_headers<<", bytes_data: "<<bytes_data<<", ini_copy+copy_length: "<<ini_copy+copy_length<<"\n";
		vector_bytes_headers.push_back(bytes_headers);
		vector_bytes_headers.push_back(bytes_data);
		total_bytes_data += bytes_data;
		
		length -= copy_length;
//		text += copy_length;
		total_copied_chars += copy_length;
		if( length < 1 ){
			break;
		}
		
	}//for... cada bloque
	file_headers.close();
	file_data.close();
	
	// Merge de archivos (ojo con el header que puede ser mas grande)
	
	cout<<"CompressorSingleBuffer::write - merge de headers\n";
	// Headers
	// Notar que aqui podria necesitar un BlockHeaders::reloadBlock() que reemplace sus valores
	headers->unprepare(block_ini);
	file_headers.open(path_headers, fstream::binary | fstream::in);
	if( (! file_headers.good()) || (! file_headers.is_open()) ){
		cerr<<"CompressorSingleBuffer::write - Error en lectura de \""<<path_headers<<"\"\n";
		delete [] full_buffer;
		delete [] text;
		return 0;
	}
	for( block = block_ini; block <= block_fin; ++block ){
		headers->reloadBlock(&file_headers, bytes_headers, block);
	}
	file_headers.close();
	headers->prepare();
	cout<<"CompressorSingleBuffer::write - n_blocks final: "<<headers->getNumBlocks()<<"\n";
	fstream file_merge(path_merge, fstream::trunc | fstream::binary | fstream::out);
	cout<<"CompressorSingleBuffer::write - BlockHeadersFactory::save...\n";
	BlockHeadersFactory::save(headers, &file_merge);
	
	// Data
	// Primero copiar datos desde bloque [0, block_ini)
	// Luego copiar datos del archivo nuevo (desde [block_ini, block_fin])
	// Luego copiar (si hay) datos desde (block_fin, n_blocks] desde el original
	unsigned int copied = 0;
	unsigned int this_copy = 0;
	
	// Primera copia [0, block_ini) (excluyente, me basta con los margenes)
	cout<<"CompressorSingleBuffer::write - Primera copia de data (copiar?: "<<(block_ini > 0)<<")\n";
	if( block_ini > 0 ){
		// Desde getDataPosition() hasta getBlockPosition(block_ini)
		// Esos datos los guardo en first_copy_start y first_copy_end
		file_data.open(master_file, fstream::binary | fstream::in);
		file_data.seekg(first_copy_start, file_data.beg);
		unsigned int total = first_copy_end - first_copy_start;
		copied = 0;
		this_copy = full_buffer_size;
		while( copied < total ){
			if( copied + this_copy > total ){
				this_copy = total - copied;
			}
			file_data.read(full_buffer, this_copy);
			file_merge.write(full_buffer, this_copy);
			copied += this_copy;
		}
		file_data.close();
	}
	
	// Segunda copia (archivo nuevo completo)
	cout<<"CompressorSingleBuffer::write - Segunda copia (total_bytes_data: "<<total_bytes_data<<")\n";
	file_data.open(path_data, fstream::binary | fstream::in);
	copied = 0;
	this_copy = full_buffer_size;
	while( copied < total_bytes_data ){
		if( copied + this_copy > total_bytes_data ){
			this_copy = total_bytes_data - copied;
		}
		file_data.read(full_buffer, this_copy);
		file_merge.write(full_buffer, this_copy);
		copied += this_copy;
	}
	file_data.close();
	
	// Tercera copia (resto del archivo original)
	cout<<"CompressorSingleBuffer::write - Tercera copia (copiar?: "<<(third_copy_start > 0)<<")\n";
	if( (third_copy_start > 0) && (third_copy_end > 0) ){
		file_data.open(master_file, fstream::binary | fstream::in);
		file_data.seekg(third_copy_start, file_data.beg);
		unsigned int total = third_copy_end - third_copy_start;
		copied = 0;
		this_copy = full_buffer_size;
		while( copied < total ){
			if( copied + this_copy > total ){
				this_copy = total - copied;
			}
			file_data.read(full_buffer, this_copy);
			file_merge.write(full_buffer, this_copy);
			copied += this_copy;
		}
		file_data.close();
	}
	
	cout<<"CompressorSingleBuffer::write - file_merge.close()\n";
	file_merge.close();
	cout<<"CompressorSingleBuffer::write - delete full_buffer\n";
	delete [] full_buffer;
	cout<<"CompressorSingleBuffer::write - delete text\n";
	delete [] text;
	
	// Rename del archivo merge
	cout<<"CompressorSingleBuffer::write - rename (\""<<path_merge<<"\", \""<<master_file<<"\")\n";
//	remove(master_file);
	rename(path_merge, master_file);
	
	// Reload del decoder
	cout<<"CompressorSingleBuffer::write - reload...\n";
	reloadDecoder();
	
	cout<<"CompressorSingleBuffer::write - Fin (total_copied_chars: "<<total_copied_chars<<", original: "<<original_length<<")\n";
	
	return original_length;
}












