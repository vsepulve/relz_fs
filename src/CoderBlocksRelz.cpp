#include "CoderBlocksRelz.h"

CoderBlocksRelz::CoderBlocksRelz(const ReferenceIndex *_referencia){
	referencia = _referencia;
	if(referencia == NULL){
		cerr<<"CoderBlocksRelz - Advertencia, referencia NULL\n";
	}
}

CoderBlocksRelz::~CoderBlocksRelz(){
}

unsigned int CoderBlocksRelz::codingBufferSize(unsigned int block_size){
	return 2 * (block_size + 1) * sizeof(int);
}

void CoderBlocksRelz::codeBlock(const char *text, unsigned int text_size, fstream *file_headers, fstream *file_data, unsigned int &bytes_headers, unsigned int &bytes_data, char *full_buffer){
	
	if(file_headers == NULL || file_data == NULL || (! file_headers->good()) || (! file_data->good()) || referencia == NULL){
		cerr<<"CoderBlocksRelz::codeBlock - Error en fstreams\n";
		return;
	}
	if(text == NULL || text_size == 0){
		return;
	}
	bytes_headers = 0;
	bytes_data = 0;
	
	//Buffers locales (a partes del buffer entregado)
	unsigned int *buff_pos = (unsigned int *)full_buffer;
	unsigned int *buff_len = (buff_pos + text_size + 1);
	//Variables para la compresion
	unsigned int compressed_text = 0;
	unsigned int pos_prefijo = 0;
	unsigned int largo_prefijo = 0;
	unsigned int n_factores = 0;
	unsigned int max_pos = 0;
	
//	cout<<"CoderBlocksRelz::codeBlock - Text: \""<<string(text, (text_size>10)?10:text_size)<<((text_size>10)?"...":"")<<"\"\n";
	
	while(text_size > 0){
//		cout<<"CoderBlocksRelz::codeBlock - referencia->find...\n";

		referencia->find(text + compressed_text, text_size, pos_prefijo, largo_prefijo);
		
		// PRUEBA DE LARGO MAXIMO (100)
//		referencia->find(text + compressed_text, (text_size<100)?text_size:100, pos_prefijo, largo_prefijo);
		
		if(largo_prefijo == 0){
			cout<<"CoderBlocksRelz::codeBlock - Error - Prefijo de largo 0, saliendo\n";
			return;
		}
		text_size -= largo_prefijo;
		compressed_text += largo_prefijo;
		if(pos_prefijo > max_pos){
			max_pos = pos_prefijo;
		}
//		cout<<"factor\t"<<n_factores<<"\t"<<pos_prefijo<<"\t"<<largo_prefijo<<"\n";
//		cout<<"("<<pos_prefijo<<", "<<largo_prefijo<<")\n";
//		cout<<"factor_length\t"<<largo_prefijo<<"\n";
		buff_pos[n_factores] = pos_prefijo;
		buff_len[n_factores] = largo_prefijo;
		++n_factores;
		
	}//while... procesar factores
	
//	cout<<"CoderBlocksRelz::codeBlock - Escribiendo datos\n";
	
	//Escribir datos
	unsigned int bytes_pos = inner_pos_coder.encodeBlockMaxBits(buff_pos, n_factores, inner_utils.n_bits(max_pos), file_data);
	unsigned int bytes_len = inner_len_coder.encodeBlockGolomb(buff_len, n_factores, file_data);
	bytes_data = bytes_pos + bytes_len;
	
//	cout<<"CoderBlocksRelz::codeBlock - Escribiendo header\n";
	
	//Escribir header
	BlockHeadersRelz::HeaderRelz header(n_factores, bytes_pos, bytes_len);
	header.save(file_headers);
	bytes_headers = header.size();
	
}

CoderBlocks* CoderBlocksRelz::copy() const{
	CoderBlocks *coder_copy = new CoderBlocksRelz(referencia);
	return (CoderBlocks*)coder_copy;
}










