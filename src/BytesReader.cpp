#include "BytesReader.h"
	
BytesReader::BytesReader(const char *_bytes){
	bytes = bytes_ini = _bytes;
}

BytesReader::~BytesReader(){}

void BytesReader::read(char *salida, unsigned int n_bytes){
//	cout<<"BytesReader::read - n_bytes: "<<n_bytes<<", bytes[0]: "<<(unsigned int)bytes[0]<<"\n";
	memcpy(salida, bytes, n_bytes);
	bytes += n_bytes;
}

void BytesReader::seekg(unsigned int pos, unsigned int offset){
	bytes = bytes_ini + pos + offset;
}

void BytesReader::close(){
	bytes = bytes_ini = NULL;
}

bool BytesReader::good() const{
	//Tambien habria ue verificar la posicion con respecto a n_bytes
	//Quizas valga la pena agregar eso en otra version
	if( bytes != NULL && bytes_ini != NULL){
		return true;
	}
	return false;
}

