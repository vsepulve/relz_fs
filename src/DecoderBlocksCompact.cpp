#include "DecoderBlocksCompact.h"

DecoderBlocksCompact::DecoderBlocksCompact(){
	buff_size = 1024;
	buff_pos = new unsigned int[buff_size];
	buff_len = new unsigned int[buff_size];
	cur_block = 0xffffffff;
	
	headers = NULL;
	pos_coder = NULL;
	len_coder = NULL;
	
}

DecoderBlocksCompact::DecoderBlocksCompact(const char *master_file, CompactSequence *_texto){
	buff_size = 1024;
	buff_pos = new unsigned int[buff_size];
	buff_len = new unsigned int[buff_size];
	cur_block = 0xffffffff;
	
	unsigned int byte_ini_headers = 0;
	unsigned int byte_ini_pos = 0;
	unsigned int byte_ini_len = 0;
	
	fstream lector(master_file, fstream::binary | fstream::in);
	lector.read((char*)(&byte_ini_headers), sizeof(int));
	lector.read((char*)(&byte_ini_pos), sizeof(int));
	lector.read((char*)(&byte_ini_len), sizeof(int));
	lector.close();
	
	cout<<"DecoderBlocksCompact - Cargando desde posiciones ("<<byte_ini_headers<<", "<<byte_ini_pos<<", "<<byte_ini_len<<")\n";
	
	headers = new BlockHeaders();
	headers->open(master_file, byte_ini_headers);
	
	pos_coder = new PositionsCoderBlocks();
	pos_coder->open(master_file, byte_ini_pos);
	
	len_coder = new LengthsCoderBlocks();
	len_coder->open(master_file, byte_ini_len);
	
	block_size = headers->getBlockSize();
	
	texto = _texto;
	
}

DecoderBlocksCompact::~DecoderBlocksCompact(){
	if(buff_pos != NULL){
		delete [] buff_pos;
		buff_pos = NULL;
	}
	if(buff_len != NULL){
		delete [] buff_len;
		buff_len = NULL;
	}
	buff_size = 0;
	cur_block = 0xffffffff;
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

void DecoderBlocksCompact::decodeBlock(unsigned int block){
//	cout<<"DecoderBlocksCompact::decodeBlock - Inicio ("<<block<<")\n";
	
	if( (cur_block == block) || (block >= headers->getNumBlocks()-1) ){
		return;
	}
	
	cur_block_ini = block * headers->getBlockSize();
	cur_block_factores = headers->getFactors(block);
	
	if(buff_size < cur_block_factores + 1){
		delete [] buff_pos;
		buff_pos = NULL;
		delete [] buff_len;
		buff_len = NULL;
		buff_size = cur_block_factores + 1;
		buff_pos = new unsigned int[buff_size];
		buff_len = new unsigned int[buff_size];
	}
	
	//Aqui es seguro que hay al menos un bloque mas (el bloque vacio del final, por ejemplo)
	
	pos_coder->decodeBlockMaxBits(
//	pos_coder->decodeBlockVarByte(
		headers->getBytesPos(block), 
		headers->getBytesPos(block+1) - headers->getBytesPos(block), 
		headers->getFactors(block), 
		buff_pos);
	
	len_coder->decodeBlockGolomb(
		headers->getBytesLen(block), 
		headers->getBytesLen(block+1) - headers->getBytesLen(block), 
		headers->getFactors(block), 
		buff_len);
	
//	cout<<"DecoderBlocksCompact::decodeBlock - Revisando "<<cur_block_factores<<" factores\n";
//	for(unsigned int i = 0; i < cur_block_factores; ++i){
//		cout<<"("<<buff_pos[i]<<", "<<buff_len[i]<<")\n";
//	}
	
	cur_block = block;
}

void DecoderBlocksCompact::testBlocks(){
	cout<<"DecoderBlocksCompact::testBlocks - Inicio\n";
	for(unsigned int i = 0; i < headers->getNumBlocks(); ++i){
		cout<<"DecoderBlocksCompact::testBlocks - Probando block["<<i<<"]\n";
		decodeBlock(i);
	}
	cout<<"DecoderBlocksCompact::testBlocks - Fin\n";
}

unsigned int DecoderBlocksCompact::decodeText(unsigned int pos_ini, unsigned int length, char *salida){
	
//	cout<<"DecoderBlocksCompact::decodeText - Inicio ("<<pos_ini<<", "<<length<<")\n";
	
	//otra forma de verlo es "cuando hay que decodificar un nuevo bloque"
	
	//Verificacion del final
	//Una forma de verlo es tener el total del texto a parte
	if( pos_ini >= headers->getTextSize() ){
//		cout<<"DecoderBlocksCompact::decodeText - Fuera de rango, saliendo\n";
		salida[0] = 0;
		return 0;
	}
	
	//Si el numero de caracteres por bloque es fijo, se puede determinar el bloque inicial en tiempo cte
	
	unsigned int block = pos_ini / block_size;
	unsigned int block_fin = (pos_ini + length) / block_size;
	if(block_fin > headers->getNumBlocks()-2){
		block_fin = headers->getNumBlocks()-2;
	}
	
//	cout<<"DecoderBlocksCompact::decodeText - bloques: ["<<block<<", "<<block_fin<<"]\n";
	
	//pos en el factor actual de inicio de la copia
	//Normalmente es 0, solo es diferente en el primer factor
	unsigned int pos_ini_fac = 0;
	//total de caracteres copiados
	unsigned int copied_chars = 0;
	//chars a copiar del factor actual
	unsigned int copy_length = 0;
	
	//Posicion de inicio del proximo factor (al principio de cada bloque, es el inicio de bloque)
	unsigned int cur_pos_ini = 0;
	
	bool lectura_iniciada = false;
	
	for( ; block <= block_fin; ++block ){
		decodeBlock(block);
		
		//Notar que esto es (block * block_size) incluso para el ultimo
		cur_pos_ini = cur_block_ini;
		
//		cout<<"DecoderBlocksCompact::decodeText - Procesando "<<cur_block_factores<<" factores (cur_pos_ini: "<<cur_pos_ini<<")\n";
		
		for(unsigned int i = 0; i < cur_block_factores; ++i){
			cur_pos_ini += buff_len[i];
			if(cur_pos_ini < pos_ini){
				continue;
			}
//			cout<<"DecoderBlocksCompact::decodeText - usando factor "<<i<<"\n";
			//Aqui necesariamente el factor actual va (al menos en parte) a la salida
			if(lectura_iniciada){
				pos_ini_fac = 0;
			}
			else{
				pos_ini_fac = pos_ini - (cur_pos_ini - buff_len[i]);
				lectura_iniciada = true;
			}
			//Verificar el largo (restante) del factor
			if( length < (buff_len[i] - pos_ini_fac) ){
				copy_length = length;
			}
			else{
				copy_length = (buff_len[i] - pos_ini_fac);
			}
			//copiar texto del factor
//			cout<<"DecoderBlocksCompact::decodeText - Copiando "<<copy_length<<" bytes en salida["<<copied_chars<<"] desde texto["<<(buff_pos[i] + pos_ini_fac)<<"], pos_ini_fac: "<<pos_ini_fac<<"\n";
//			cout<<"string: \""<<string(texto + buff_pos[i] + pos_ini_fac, copy_length)<<"\"\n";
//			memcpy( salida + copied_chars, texto + buff_pos[i] + pos_ini_fac, copy_length );
			for(unsigned int j = 0; j < copy_length; ++j){
				*(salida + copied_chars + j) = texto->getChar(buff_pos[i] + pos_ini_fac + j);
			}
			length -= copy_length;
			copied_chars += copy_length;
			if( length < 1 ){
				break;
			}
		}//for... cada factor
		
	}//for... cada bloque
	
	salida[copied_chars] = 0;
	
//	cout<<"DecoderBlocksCompact::decodeText - Fin (copied_chars: "<<copied_chars<<")\n";
	
	return copied_chars;
}











