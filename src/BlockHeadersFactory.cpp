#include "BlockHeadersFactory.h"

BlockHeadersFactory::BlockHeadersFactory(){
}

BlockHeadersFactory::~BlockHeadersFactory(){
}

// Lee el tipo
// Construye un BlockHeaders adecuado
// Lo carga (headers->load)
// Lo retorna
BlockHeaders *BlockHeadersFactory::load(fstream *reader){
	if( reader == NULL|| (!reader->good()) ){
		cerr<<"BlockHeadersFactory::load - Error en reader\n";
		return NULL;
	}
	BlockHeaders *headers = NULL;
	char type = 0;
	reader->read(&type, 1);
	switch(type){
		case 1 : 
			headers = new BlockHeadersRelz();
			headers->load(reader);
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
	fstream reader(file_name, fstream::binary | fstream::in);
	if( reader.is_open() ){
		headers = load(&reader);
		reader.close();
	}
	return headers;
}

// Igual al anterior, pero a partir un BytesReader
BlockHeaders *BlockHeadersFactory::load(BytesReader *reader){
	if( reader == NULL|| (!reader->good()) ){
		cerr<<"BlockHeadersFactory::load - Error en reader\n";
		return NULL;
	}
	BlockHeaders *headers = NULL;
	char type = 0;
	reader->read(&type, 1);
	switch(type){
		case 1 : 
			headers = new BlockHeadersRelz();
			headers->load(reader);
		break;
		default :
			cerr<<"BlockHeadersFactory::load - Tipo desconocido ("<<(unsigned int)type<<")\n";
		break;
	}
	return headers;
}

// Verifica el tipo del header
// Escribe el tipo
// guarda headers en el writer
void BlockHeadersFactory::save(BlockHeaders *headers, fstream *writer){
	if( writer == NULL|| (!writer->good()) ){
		cerr<<"BlockHeadersFactory::save - Error en writer\n";
		return;
	}
	char type = 0;
	if( dynamic_cast<BlockHeadersRelz*>(headers) != NULL ){
		type = 1;
//		cout<<"BlockHeadersFactory::save - Escribiendo marca\n";
		writer->write(&type, 1);
//		cout<<"BlockHeadersFactory::save - headers->save...\n";
		headers->save(writer);
	}
	else{
//		cout<<"BlockHeadersFactory::save - Tipo desconocido\n";
		type = 0;
		writer->write(&type, 1);
	}
}

// Retorna el numero de bytes de type
unsigned int BlockHeadersFactory::typeSize(){
	return 1;
}












