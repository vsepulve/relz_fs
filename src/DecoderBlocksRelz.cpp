#include "DecoderBlocksRelz.h"

DecoderBlocksRelz::DecoderBlocksRelz(const char *_texto_ref){
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
}

DecoderBlocksRelz::DecoderBlocksRelz(const char *_master_file, const char *_texto_ref){
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
	
	load(_master_file);
}

DecoderBlocksRelz::~DecoderBlocksRelz(){
	deleteBuffers();
	deleteData();
}

void DecoderBlocksRelz::deleteBuffers(){
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
	
void DecoderBlocksRelz::deleteData(){
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

void DecoderBlocksRelz::prepareBuffer(unsigned int new_size){
	if(new_size > buff_size){
		deleteBuffers();
		buff_size = new_size;
		buff_pos = new unsigned int[buff_size];
		buff_len = new unsigned int[buff_size];
	}
}

void DecoderBlocksRelz::load(const char *_master_file){
	cout<<"DecoderBlocksRelz::load - Inicio ("<<_master_file<<")\n";
	
	//Borrado de datos previos antes de cargar
	deleteData();
	master_file = _master_file;
	
	headers = BlockHeadersFactory::load(master_file);
	if(headers == NULL){
		cout<<"DecoderBlocksRelz::load - Imposible cargar headers, saliendo\n";
		return;
	}
	
	unsigned int byte_ini_data = headers->getDataPosition();
	cout<<"DecoderBlocksRelz::load - Cargando datos desde "<<byte_ini_data<<"\n";
	
	pos_coder = new PositionsCoderBlocks();
	pos_coder->open(master_file, byte_ini_data);
	
	len_coder = new LengthsCoderBlocks();
	len_coder->open(master_file, byte_ini_data);
	
}

unsigned int DecoderBlocksRelz::decodeBlock(unsigned int block, char *buff){
//	cout<<"DecoderBlocksRelz::decodeBlock - Inicio ("<<block<<", n_blocks: "<<headers->getNumBlocks()<<")\n";
	
	if( headers == NULL || texto_ref == NULL || (block >= headers->getNumBlocks())){
		buff[0] = 0;
		return 0;
	}
	BlockHeadersRelz *headers_relz = NULL;
	if( (headers_relz = dynamic_cast<BlockHeadersRelz*>(headers)) == NULL ){
		cerr<<"DecoderBlocksRelz::decodeBlock - Error, Headers de tipo incorrecto\n";
		buff[0] = 0;
		return 0;
	}
	
	bool extraer_factores = true;
	if( cur_block == block ){
		extraer_factores = false;
	}
	
//	cout<<"DecoderBlocksRelz::decodeBlock - extraer_factores: "<<extraer_factores<<"\n";
	
	//Procesar pos y len en buffers internos
	if(extraer_factores){
//		cout<<"DecoderBlocksRelz::decodeBlock - headers_relz->getFactors...\n";
		cur_block_factores = headers_relz->getFactors(block);
		
		prepareBuffer(cur_block_factores + 1);
		memset(buff_pos, 0, (cur_block_factores + 1) * sizeof(int));
		memset(buff_len, 0, (cur_block_factores + 1) * sizeof(int));
		
		//Aqui es seguro que hay al menos un bloque mas (el bloque vacio del final, por ejemplo)
		
//		cout<<"DecoderBlocksRelz::decodeBlock - pos_coder->decodeBlockMaxBits...\n";
		pos_coder->decodeBlockMaxBits(
			headers_relz->getBytesPos(block), 
			headers_relz->getBytesLen(block) - headers_relz->getBytesPos(block), 
			headers_relz->getFactors(block), 
			buff_pos);
		
//		cout<<"DecoderBlocksRelz::decodeBlock - len_coder->decodeBlockGolomb...\n";
		len_coder->decodeBlockGolomb(
			headers_relz->getBytesLen(block), 
			headers_relz->getBytesPos(block+1) - headers_relz->getBytesLen(block), 
			headers_relz->getFactors(block), 
			buff_len);
		
//		cout<<"DecoderBlocksRelz::decodeBlock - Revisando "<<cur_block_factores<<" factores\n";
//		for(unsigned int i = 0; i < cur_block_factores; ++i){
//			cout<<"("<<buff_pos[i]<<", "<<buff_len[i]<<")\n";
//		}
		
		cur_block = block;
	}
	
//	cout<<"DecoderBlocksRelz::decodeBlock - Extrayendo texto\n";
	//extraer texto de los factores
	unsigned int copied_chars = 0;
	for(unsigned int i = 0; i < cur_block_factores; ++i){
//		cout<<"DecoderBlocksRelz::decodeBlock - memcpy (buff["<<copied_chars<<"], texto_ref["<<buff_pos[i]<<"], "<<buff_len[i]<<")\n";
		memcpy( buff + copied_chars, texto_ref + buff_pos[i], buff_len[i] );
		copied_chars += buff_len[i];
	}
	buff[copied_chars] = 0;
	
//	cout<<"DecoderBlocksRelz::decodeBlock - Fin ("<<copied_chars<<" chars copiados, \""<<buff<<"\")\n";
	return copied_chars;
}
	
//Este metodo retorna un NUEVO objeto headers del tipo correcto
//Usa los argumentos en la creacion del header
BlockHeaders *DecoderBlocksRelz::getNewHeaders(unsigned long long _text_size, unsigned int _block_size, Metadata *_metadata){
	return new BlockHeadersRelz(_text_size, _block_size, _metadata);
}

DecoderBlocks* DecoderBlocksRelz::copy() const{
	DecoderBlocks *decoder_copy = new DecoderBlocksRelz(texto_ref);
	return (DecoderBlocks*)decoder_copy;
}

DecoderBlocks* DecoderBlocksRelz::copyBytes(const char *bytes) const{
	DecoderBlocks *decoder_copy = new DecoderBlocksRelzBytes(bytes, texto_ref);
//	decoder_copy->load(bytes);
	return (DecoderBlocks*)decoder_copy;
}








