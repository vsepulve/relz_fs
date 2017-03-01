#include "BlockHeadersFactory.h"

BlockHeadersFactory::BlockHeadersFactory(){
}

BlockHeadersFactory::~BlockHeadersFactory(){
}

// Lee el tipo
// Construye un BlockHeaders adecuado
// Lo carga (headers->load)
// Lo retorna
BlockHeaders *BlockHeadersFactory::load(fstream *lector){
	if( lector == NULL|| (!lector->good()) ){
		cerr<<"BlockHeadersFactory::load - Error en lector\n";
		return NULL;
	}
	BlockHeaders *headers = NULL;
	char type = 0;
	lector->read(&type, 1);
	switch(type){
		case 1 : 
			headers = new BlockHeadersRelz();
			headers->load(lector);
		break;
		default :
			cerr<<"BlockHeadersFactory::load - Tipo desconocido ("<<(unsigned int)type<<")\n";
		break;
	}
	return headers;
}

// Igual al anterior, pero a partir del nombre del archivo
BlockHeaders *BlockHeadersFactory::load(const char *file_name){
	BlockHeaders *headers = NULL;
	fstream lector(file_name, fstream::binary | fstream::in);
	if( lector.is_open() ){
		headers = load(&lector);
		lector.close();
	}
	return headers;
}

// Igual al anterior, pero a partir un BytesReader
BlockHeaders *BlockHeadersFactory::load(BytesReader *lector){
	if( lector == NULL|| (!lector->good()) ){
		cerr<<"BlockHeadersFactory::load - Error en lector\n";
		return NULL;
	}
	BlockHeaders *headers = NULL;
	char type = 0;
	lector->read(&type, 1);
	switch(type){
		case 1 : 
			headers = new BlockHeadersRelz();
			headers->load(lector);
		break;
		default :
			cerr<<"BlockHeadersFactory::load - Tipo desconocido ("<<(unsigned int)type<<")\n";
		break;
	}
	return headers;
}

// Verifica el tipo del header
// Escribe el tipo
// guarda headers en el escritor
void BlockHeadersFactory::save(BlockHeaders *headers, fstream *escritor){
	if( escritor == NULL|| (!escritor->good()) ){
		cerr<<"BlockHeadersFactory::save - Error en escritor\n";
		return;
	}
	char type = 0;
	if( dynamic_cast<BlockHeadersRelz*>(headers) != NULL ){
		type = 1;
//		cout<<"BlockHeadersFactory::save - Escribiendo marca\n";
		escritor->write(&type, 1);
//		cout<<"BlockHeadersFactory::save - headers->save...\n";
		headers->save(escritor);
	}
	else{
//		cout<<"BlockHeadersFactory::save - Tipo desconocido\n";
		type = 0;
		escritor->write(&type, 1);
	}
}

// Retorna el numero de bytes de type
unsigned int BlockHeadersFactory::typeSize(){
	return 1;
}












