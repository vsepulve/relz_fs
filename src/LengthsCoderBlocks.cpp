#include "LengthsCoderBlocks.h"

LengthsCoderBlocks::LengthsCoderBlocks(){
	nombre_archivo = NULL;
	buff_size = 0;
	buff = NULL;
	byte_ini = 0;
}

LengthsCoderBlocks::LengthsCoderBlocks(const char *_nombre_archivo){
	nombre_archivo = _nombre_archivo;
	buff_size = 0;
	buff = NULL;
	byte_ini = 0;
}

LengthsCoderBlocks::~LengthsCoderBlocks(){
	deleteBuffer();
}

void LengthsCoderBlocks::deleteBuffer(){
	if(buff != NULL){
		delete [] buff;
		buff = NULL;
		buff_size = 0;
	}
}

void LengthsCoderBlocks::prepareBuffer(unsigned int new_size){
	if( new_size > buff_size ){
		deleteBuffer();
		buff_size = new_size;
		buff = new unsigned int[buff_size];
	}
}

void LengthsCoderBlocks::setGolombBase(unsigned int potencia_base){
	utils.setGolombBase(potencia_base);
}

unsigned int LengthsCoderBlocks::encodeBlockGolomb(unsigned int *arr_len, unsigned int n_factores, fstream *escritor){
	
//	cout<<"LengthsCoderBlocks::encodeBlockGolomb - Inicio ("<<n_factores<<" largos)\n";
	//En esta version primero calculare el numero real de bits
	//Con eso, determino el numero de bytes real a ser usado
	unsigned long long total_bits = 0;
	for(unsigned int i = 0; i < n_factores; ++i){
		total_bits += utils.bits_golomb(arr_len[i]);
	}
	//cambio a ints
	total_bits >>= 5;
	//Agrego 2 por simplicidad, 1 por el redondeo de la division, y otro para un potencial 0 final
	total_bits += 2;
	prepareBuffer(total_bits);
//	memset(buff, 0, total_bits * sizeof(int));
	memset(buff, 0, (total_bits << 2));
	unsigned int pos_buff = 0;
	for(unsigned int i = 0; i < n_factores; ++i){
		pos_buff += utils.write_golomb(buff, pos_buff, arr_len[i]);
	}
//	unsigned int n_escribir = pos_buff / 32;
//	if(n_escribir * 32 < pos_buff){
//		++n_escribir;
//	}
	unsigned int n_escribir = (pos_buff >> 5);
	if( (n_escribir << 5) < pos_buff ){
		++n_escribir;
	}
//	cout<<"LengthsCoderBlocks::encodeBlockGolomb - Escribiendo archivo ("<<n_escribir<<" enteros)\n";
	escritor->write((char*)buff, n_escribir * sizeof(int));
//	cout<<"LengthsCoderBlocks::encodeBlockGolomb - Fin (byte "<<archivo->tellp()<<")\n";
	return n_escribir * sizeof(int);
}

//Metodo de Lectura
void LengthsCoderBlocks::open(const char *_nombre_archivo, unsigned int _byte_ini){
	cout<<"LengthsCoderBlocks::open - inicio (\""<<_nombre_archivo<<"\" desde byte "<<_byte_ini<<")\n";
	//En lugar de hacer un open explicito (que mantenga abierto el stream) solo lo trato de ese modo
	//Por seguridad, abro el archivo solo al leerlo (quizas pueda recibirse un stream abierto por el llamador)
	nombre_archivo = _nombre_archivo;
	byte_ini = _byte_ini;
}

void LengthsCoderBlocks::decodeBlockGolomb(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_len){
//	cout<<"LengthsCoderBlocks::decodeBlockGolomb - Inicio (inicio: "<<byte_start<<", bytes: "<<n_bytes<<", n_factores: "<<n_factores<<")\n";
	unsigned int max_ints = 1 + (n_bytes >> 2);
	prepareBuffer(max_ints);
//	prepareBuffer(n_factores + 1);
	memset(buff, 0, max_ints * sizeof(int));
	fstream archivo(nombre_archivo, fstream::binary | fstream::in);
	if( ! archivo.good() ){
//		cerr<<"LengthsCoderBlocks::decodeBlockGolomb - Error al abrir \""<<nombre_archivo<<"\"\n";
		return;
	}
	archivo.seekg(byte_start + byte_ini, archivo.beg);
	archivo.read((char*)buff, n_bytes);
	archivo.close();
	unsigned int pos_buff = 0;
	for(unsigned int i = 0; i < n_factores; ++i){
		pos_buff += utils.read_golomb(buff, pos_buff, arr_len[i]);
	}
//	cout<<"LengthsCoderBlocks::decodeBlockGolomb - Fin\n";
}






