#include "DecoderBlocksRelzBytes.h"

DecoderBlocksRelzBytes::DecoderBlocksRelzBytes(const char *_texto_ref){
	buff_size = 0;
	buff_pos = NULL;
	buff_len = NULL;
	cur_block = 0xffffffff;
	cur_block_factores = 0;
	
	headers = NULL;
	pos_coder = NULL;
	len_coder = NULL;
	texto_ref = _texto_ref;
	if(texto_ref == NULL){
		cerr<<"DecoderBlocksRelzBytes - Advertencia, texto_ref NULL\n";
	}
}

DecoderBlocksRelzBytes::DecoderBlocksRelzBytes(const char *bytes, const char *_texto_ref){
	buff_size = 0;
	buff_pos = NULL;
	buff_len = NULL;
	cur_block = 0xffffffff;
	cur_block_factores = 0;
	
	headers = NULL;
	pos_coder = NULL;
	len_coder = NULL;
	texto_ref = _texto_ref;
	if(texto_ref == NULL){
		cerr<<"DecoderBlocksRelz - Advertencia, texto_ref NULL\n";
	}
	
	load(bytes);
	
}

DecoderBlocksRelzBytes::~DecoderBlocksRelzBytes(){
	deleteBuffers();
	deleteData();
}

void DecoderBlocksRelzBytes::deleteBuffers(){
	if(buff_pos != NULL){
		delete [] buff_pos;
		buff_pos = NULL;
	}
	if(buff_len != NULL){
		delete [] buff_len;
		buff_len = NULL;
	}
	buff_size = 0;
}
	
void DecoderBlocksRelzBytes::deleteData(){
	cur_block = 0xffffffff;
	cur_block_factores = 0;
	if(headers != NULL){
		delete headers;
		headers = NULL;
	}
	if(pos_coder != NULL){
		delete pos_coder;
		pos_coder = NULL;
	}
	if(len_coder != NULL){
		delete len_coder;
		len_coder = NULL;
	}
}

void DecoderBlocksRelzBytes::prepareBuffer(unsigned int new_size){
	if(new_size > buff_size){
		deleteBuffers();
		buff_size = new_size;
		buff_pos = new unsigned int[buff_size];
		buff_len = new unsigned int[buff_size];
	}
}

void DecoderBlocksRelzBytes::load(const char *bytes){
	cout<<"DecoderBlocksRelzBytes::load - Inicio\n";
	
	//Borrado de datos previos antes de cargar
	deleteData();
	
	BytesReader lector(bytes);
	headers = BlockHeadersFactory::load(&lector);
	if(headers == NULL){
		cout<<"DecoderBlocksRelzBytes::load - Imposible cargar headers, saliendo\n";
		return;
	}
	
	unsigned int byte_ini_data = headers->getDataPosition();
	cout<<"DecoderBlocksRelzBytes::load - Cargando datos desde "<<byte_ini_data<<"\n";
	
	cout<<"DecoderBlocksRelzBytes::load - new pos_coder...\n";
	pos_coder = new PositionsCoderBlocksBytes();
	cout<<"DecoderBlocksRelzBytes::load - pos_coder->open...\n";
	pos_coder->open(bytes + byte_ini_data);
	
	cout<<"DecoderBlocksRelzBytes::load - new len_coder...\n";
	len_coder = new LengthsCoderBlocksBytes();
	cout<<"DecoderBlocksRelzBytes::load - len_coder->open...\n";
	len_coder->open(bytes + byte_ini_data);
	
	cout<<"DecoderBlocksRelzBytes::load - Fin\n";
}

unsigned int DecoderBlocksRelzBytes::decodeBlock(unsigned int block, char *buff){
	cout<<"DecoderBlocksRelzBytes::decodeBlock - Inicio ("<<block<<")\n";
	
	//Recordar que headers tiene 1 bloque mas de los reales (con la posicion final de lectura)
	if( headers == NULL || texto_ref == NULL || (block >= headers->getNumBlocks())){
		buff[0] = 0;
		return 0;
	}
	BlockHeadersRelz *headers_relz = NULL;
	if( (headers_relz = dynamic_cast<BlockHeadersRelz*>(headers)) == NULL ){
		cerr<<"DecoderBlocksRelzBytes::decodeBlock - Error, Headers de tipo incorrecto\n";
		buff[0] = 0;
		return 0;
	}
	
	if(block != cur_block){
		cur_block_factores = headers_relz->getFactors(block);
		
		prepareBuffer(cur_block_factores + 1);
		memset(buff_pos, 0, (cur_block_factores + 1) * sizeof(int));
		memset(buff_len, 0, (cur_block_factores + 1) * sizeof(int));
		
		//Aqui es seguro que hay al menos un bloque mas (el bloque vacio del final, por ejemplo)
		
		cout<<"DecoderBlocksRelzBytes::decodeBlock - pos_coder->decodeBlockMaxBits...\n";
		pos_coder->decodeBlockMaxBits(
			headers_relz->getBytesPos(block), 
			headers_relz->getBytesLen(block) - headers_relz->getBytesPos(block), 
			headers_relz->getFactors(block), 
			buff_pos);
		
		cout<<"DecoderBlocksRelzBytes::decodeBlock - len_coder->decodeBlockGolomb...\n";
		len_coder->decodeBlockGolomb(
			headers_relz->getBytesLen(block), 
			headers_relz->getBytesPos(block+1) - headers_relz->getBytesLen(block), 
			headers_relz->getFactors(block), 
			buff_len);
		
//		cout<<"DecoderBlocksRelzBytes::decodeBlock - Revisando "<<cur_block_factores<<" factores\n";
//		for(unsigned int i = 0; i < ( (cur_block_factores<3)?cur_block_factores:3 ) ; ++i){
//			cout<<"("<<buff_pos[i]<<", "<<buff_len[i]<<")\n";
//		}
		
		cur_block = block;
	}
	
	cout<<"DecoderBlocksRelzBytes::decodeBlock - Extrayendo texto\n";
	unsigned int copied_chars = 0;
	//Ahora cur_block esta valido en el buff y se puede leer
	for(unsigned int i = 0; i < cur_block_factores; ++i){
		//decode (buff_pos[i], buff_len[i])
		memcpy(buff + copied_chars, texto_ref + buff_pos[i], buff_len[i]);
		copied_chars += buff_len[i];
	}
	buff[copied_chars] = 0;
	
//	cout<<"DecoderBlocksRelzBytes::decodeBlock - Fin ("<<copied_chars<<" chars copiados, \""<<buff<<"\")\n";
	cout<<"DecoderBlocksRelzBytes::decodeBlock - Fin ("<<copied_chars<<" chars copiados)\n";
	return copied_chars;
}
	
//Este metodo retorna un NUEVO objeto headers del tipo correcto
//Usa los argumentos en la creacion del header
BlockHeaders *DecoderBlocksRelzBytes::getNewHeaders(unsigned int _text_size, unsigned int _block_size, Metadata *_metadata){
	return new BlockHeadersRelz(_text_size, _block_size, _metadata);
}



