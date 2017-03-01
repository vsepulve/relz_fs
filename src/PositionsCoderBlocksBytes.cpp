#include "PositionsCoderBlocksBytes.h"

PositionsCoderBlocksBytes::PositionsCoderBlocksBytes(){
	archivo = NULL;
	buff_size = 0;
	buff = NULL;
	byte_ini = 0;
}

PositionsCoderBlocksBytes::PositionsCoderBlocksBytes(const char *bytes){
	archivo = new BytesReader(bytes);
	buff_size = 0;
	buff = NULL;
	byte_ini = 0;
}

PositionsCoderBlocksBytes::~PositionsCoderBlocksBytes(){
	if(archivo != NULL){
		archivo->close();
		delete archivo;
		archivo = NULL;
	}
	deleteBuffer();
}

void PositionsCoderBlocksBytes::deleteBuffer(){
	if(buff != NULL){
		delete [] buff;
		buff = NULL;
		buff_size = 0;
	}
}

void PositionsCoderBlocksBytes::prepareBuffer(unsigned int new_size){
	if( new_size > buff_size ){
		deleteBuffer();
		buff_size = new_size;
		buff = new unsigned int[buff_size];
	}
}

//Metodo de Lectura
void PositionsCoderBlocksBytes::open(const char *bytes){
	cout<<"PositionsCoderBlocksBytes::open - inicio\n";
	if(archivo != NULL){
		archivo->close();
		delete archivo;
		archivo = NULL;
	}
	archivo = new BytesReader(bytes);
}

//Metodo de Lectura
//Lee el archivo (ya abierto) desde byte_start
//Comienza leyendo max_bits y luego carga  n_factores posiciones en arr_pos
//Asume que arr_pos tiene al menos (n_factores + 1) ints de espacio
void PositionsCoderBlocksBytes::decodeBlockMaxBits(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_pos){
	if(archivo == NULL){
		cerr<<"PositionsCoderBlocksBytes::decodeBlockMaxBits - Error, lector NULL\n";
		return;
	}
	cout<<"PositionsCoderBlocksBytes::decodeBlockMaxBits - Inicio (inicio: "<<byte_start<<", bytes: "<<n_bytes<<", n_factores: "<<n_factores<<")\n";
	unsigned int max_ints = 1 + (n_bytes >> 2);
	prepareBuffer(max_ints);
	memset(buff, 0, n_bytes + sizeof(int));
	archivo->seekg(byte_start + byte_ini, archivo->beg);
	archivo->read((char*)buff, n_bytes);
	unsigned int pos_buff = 0;
	unsigned char max_bits = 0;
	max_bits = utils.bitget(buff, pos_buff, 8);
	pos_buff += 8;
//	cout<<"PositionsCoderBlocksBytes::decodeBlockMaxBits - max_bits: "<<(unsigned int)max_bits<<"\n";
	for(unsigned int i = 0; i < n_factores; ++i){
		arr_pos[i] = utils.bitget(buff, pos_buff, max_bits);
		pos_buff += max_bits;
	}
	cout<<"PositionsCoderBlocksBytes::decodeBlockMaxBits - Fin\n";
}

//Metodo de Lectura
//Lee el archivo (ya abierto) desde byte_start
//Carga n_factores posiciones en arr_pos usando read_varbyte
//Asume que arr_pos tiene al menos (n_factores + 1) ints de espacio
void PositionsCoderBlocksBytes::decodeBlockVarByte(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_pos){
	if(archivo == NULL){
		cerr<<"PositionsCoderBlocksBytes::decodeBlockVarByte - Error, lector NULL\n";
		return;
	}
//	cout<<"PositionsCoderBlocksBytes::decodeBlockMaxBits - Inicio (inicio: "<<byte_start<<", bytes: "<<n_bytes<<", n_factores: "<<n_factores<<")\n";
	unsigned int max_ints = 1 + (n_bytes >> 2);
	prepareBuffer(max_ints);
	memset(buff, 0, n_bytes + sizeof(int));
	archivo->seekg(byte_start + byte_ini, archivo->beg);
	archivo->read((char*)buff, n_bytes);
	unsigned int pos_buff = 0;
	unsigned char *buff_chars = (unsigned char*)buff;
	for(unsigned int i = 0; i < n_factores; ++i){
		pos_buff += utils.read_varbyte(buff_chars + pos_buff, arr_pos[i]);
	}
//	cout<<"PositionsCoderBlocksBytes::decodeBlockMaxBits - Fin\n";
}












