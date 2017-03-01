#include "PositionsCoderBlocks.h"

PositionsCoderBlocks::PositionsCoderBlocks(){
	nombre_archivo = NULL;
	buff_size = 0;
	buff = NULL;
	byte_ini = 0;
}

PositionsCoderBlocks::PositionsCoderBlocks(const char *_nombre_archivo){
	nombre_archivo = _nombre_archivo;
	buff_size = 0;
	buff = NULL;
	byte_ini = 0;
}

PositionsCoderBlocks::~PositionsCoderBlocks(){
	deleteBuffer();
}

void PositionsCoderBlocks::deleteBuffer(){
	if(buff != NULL){
		delete [] buff;
		buff = NULL;
		buff_size = 0;
	}
}

void PositionsCoderBlocks::prepareBuffer(unsigned int new_size){
	if( new_size > buff_size ){
		deleteBuffer();
		buff_size = new_size;
		buff = new unsigned int[buff_size];
	}
}

unsigned int PositionsCoderBlocks::encodeBlockMaxBits(unsigned int *arr_pos, unsigned int n_factores, unsigned char max_bits, fstream *escritor){
	if(escritor == NULL || (! escritor->good()) ){
		return 0;
	}
	prepareBuffer(n_factores + 1);
	memset(buff, 0, (n_factores + 1) * sizeof(int));
	unsigned int pos_buff = 0;
	utils.bitput(buff, pos_buff, 8, max_bits);
	pos_buff += 8;
	for(unsigned int i = 0; i < n_factores; ++i){
		utils.bitput(buff, pos_buff, max_bits, arr_pos[i]);
		pos_buff += max_bits;
	}
//	unsigned int n_escribir = pos_buff / 32;
//	if(n_escribir * 32 < pos_buff){
//		++n_escribir;
//	}
	unsigned int n_escribir = (pos_buff >> 5);
	if( (n_escribir << 5) < pos_buff ){
		++n_escribir;
	}
//	unsigned int pos_ini = escritor->tellp();
	escritor->write((char*)buff, n_escribir * sizeof(int));
//	unsigned int pos_fin = escritor->tellp();
//	return pos_fin - pos_ini;
	return n_escribir * sizeof(int);
}

unsigned int PositionsCoderBlocks::encodeBlockVarByte(unsigned int *arr_pos, unsigned int n_factores, fstream *escritor){
	if(escritor == NULL || (! escritor->good()) ){
		return 0;
	}
//	cout<<"PositionsCoderBlocks::encodeBlockVarByte - Inicio ("<<n_factores<<" posicones)\n";
	
	//Calculo el maximo de bytes a usar, el peor caso
	unsigned max_ints = 1 + ((n_factores * 5) >> 2);
	prepareBuffer(max_ints);
//	memset(buff, 0, max_ints * sizeof(int) );
	memset(buff, 0, (max_ints << 2));
	
	//En este caso la posicion es en bytes
	unsigned int pos_buff = 0;
	unsigned char *buff_chars = (unsigned char*)buff;
	for(unsigned int i = 0; i < n_factores; ++i){
		pos_buff += utils.write_varbyte(buff_chars + pos_buff, arr_pos[i]);
	}
	
//	cout<<"PositionsCoderBlocks::encodeBlockVarByte - Escribiendo archivo ("<<pos_buff<<" bytes)\n";
	escritor->write((char*)buff_chars, pos_buff);
	
	return pos_buff;
}

//Metodo de Lectura
void PositionsCoderBlocks::open(const char *_nombre_archivo, unsigned int _byte_ini){
	cout<<"PositionsCoderBlocks::open - inicio (\""<<_nombre_archivo<<"\" desde byte "<<_byte_ini<<")\n";
	//En lugar de hacer un open explicito (que mantenga abierto el stream) solo lo trato de ese modo
	//Por seguridad, abro el archivo solo al leerlo (quizas pueda recibirse un stream abierto por el llamador)
	nombre_archivo = _nombre_archivo;
	byte_ini = _byte_ini;
}

//Metodo de Lectura
//Lee el archivo desde byte_start
//Comienza leyendo max_bits y luego carga  n_factores posiciones en arr_pos
//Asume que arr_pos tiene al menos (n_factores + 1) ints de espacio
void PositionsCoderBlocks::decodeBlockMaxBits(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_pos){
//	cout<<"PositionsCoderBlocks::decodeBlockMaxBits - Inicio (inicio: "<<byte_start<<", bytes: "<<n_bytes<<", n_factores: "<<n_factores<<")\n";
	unsigned int max_ints = 1 + (n_bytes >> 2);
	prepareBuffer(max_ints);
	memset(buff, 0, n_bytes + sizeof(int));
	fstream archivo(nombre_archivo, fstream::binary | fstream::in);
	if( ! archivo.good() ){
		cerr<<"PositionsCoderBlocks::decodeBlockMaxBits - Error al abrir \""<<nombre_archivo<<"\"\n";
		return;
	}
	archivo.seekg(byte_start + byte_ini, archivo.beg);
	archivo.read((char*)buff, n_bytes);
	archivo.close();
	unsigned int pos_buff = 0;
	unsigned char max_bits = 0;
	max_bits = utils.bitget(buff, pos_buff, 8);
	pos_buff += 8;
//	cout<<"PositionsCoderBlocks::decodeBlockMaxBits - max_bits: "<<(unsigned int)max_bits<<"\n";
	for(unsigned int i = 0; i < n_factores; ++i){
		arr_pos[i] = utils.bitget(buff, pos_buff, max_bits);
		pos_buff += max_bits;
	}
//	cout<<"PositionsCoderBlocks::decodeBlockMaxBits - Fin\n";
}

//Metodo de Lectura
//Lee el archivo desde byte_start
//Carga n_factores posiciones en arr_pos usando read_varbyte
//Asume que arr_pos tiene al menos (n_factores + 1) ints de espacio
void PositionsCoderBlocks::decodeBlockVarByte(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_pos){
//	cout<<"PositionsCoderBlocks::decodeBlockMaxBits - Inicio (inicio: "<<byte_start<<", bytes: "<<n_bytes<<", n_factores: "<<n_factores<<")\n";
	unsigned int max_ints = (n_bytes >> 2) + 1;
	prepareBuffer(max_ints);
	memset(buff, 0, n_bytes + sizeof(int));
	fstream archivo(nombre_archivo, fstream::binary | fstream::in);
	if( ! archivo.good() ){
		cerr<<"PositionsCoderBlocks::decodeBlockVarByte - Error al abrir \""<<nombre_archivo<<"\"\n";
		return;
	}
	archivo.seekg(byte_start + byte_ini, archivo.beg);
	archivo.read((char*)buff, n_bytes);
	archivo.close();
	unsigned int pos_buff = 0;
	unsigned char *buff_chars = (unsigned char*)buff;
	for(unsigned int i = 0; i < n_factores; ++i){
		pos_buff += utils.read_varbyte(buff_chars + pos_buff, arr_pos[i]);
	}
//	cout<<"PositionsCoderBlocks::decodeBlockMaxBits - Fin\n";
}












