#include "DecoderBlocks.h"

DecoderBlocks::DecoderBlocks(){
	headers = NULL;
}

DecoderBlocks::DecoderBlocks(const char *master_file){
	headers = NULL;
}

DecoderBlocks::~DecoderBlocks(){
	if(headers != NULL){
		delete headers;
		headers = NULL;
	}
}
	
void DecoderBlocks::load(const char *master_file){
	cerr<<"DecoderBlocks::load - No Implementado\n";
}

unsigned int DecoderBlocks::decodeBlock(unsigned int block, char *buff){
	cerr<<"DecoderBlocks::load - No Implementado\n";
	return 0;
}

unsigned long long DecoderBlocks::getTextSize(){
	if(headers == NULL){
		return 0;
	}
	return headers->getTextSize();
}

unsigned int DecoderBlocks::getBlockSize(){
	if(headers == NULL){
		return 0;
	}
	return headers->getBlockSize();
}

unsigned int DecoderBlocks::getNumBlocks(){
	if(headers == NULL){
		return 0;
	}
	return headers->getNumBlocks();
}
	
BlockHeaders *DecoderBlocks::getHeaders(){
	return headers;
}
	
//Este metodo retorna un NUEVO objeto headers del tipo correcto
//Usa los argumentos en la creacion del header
BlockHeaders *DecoderBlocks::getNewHeaders(unsigned long long _text_size, unsigned int _block_size, Metadata *_metadata){
	return new BlockHeaders(_text_size, _block_size, _metadata);
}

DecoderBlocks* DecoderBlocks::copy() const{
	DecoderBlocks *decoder_copy = new DecoderBlocks();
	return decoder_copy;
}

DecoderBlocks* DecoderBlocks::copyBytes(const char *bytes) const{
	//DecoderBlocks (basico) NO tiene un equivalente Bytes
	return NULL;
}







