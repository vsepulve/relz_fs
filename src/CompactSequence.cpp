#include "CompactSequence.h"

CompactSequence::CompactSequence(){}

CompactSequence::CompactSequence(char *_text, unsigned int _text_size){
	text_size = _text_size;
	n_bytes = text_size / 2;
	if(2 * n_bytes < text_size){
		++n_bytes;
	}
	cout<<"CompactSequence - Preparando "<<n_bytes<<" bytes para el texto de "<<_text_size<<" chars\n";
	bytes = new char[n_bytes];
	unsigned int pos_byte = 0;
	//cambio de byte en posiciones impares
	bool impar = false;
	char valor = 0;
	for(unsigned int i = 0; i < text_size; ++i){
		switch(_text[i]){
			case 'A':
				valor = 1;
				break;
			case 'C':
				valor = 2;
				break;
			case 'G':
				valor = 3;
				break;
			case 'T':
				valor = 4;
				break;
			case 'N':
				valor = 5;
				break;
			default :
				valor = 0;
				break;
		}
		if(impar){
			bytes[pos_byte] |= ( (valor << 4) & 0xf0);
			++pos_byte;
		}
		else{
			bytes[pos_byte] = (valor & 0x0f);
		}
		impar = (!impar);
	}
	
	cout<<"CompactSequence - Compactacion terminada\n";
	
//	for(unsigned int i = 0; i < 10; ++i){
//		cout<<"["<<i<<"] - "<<_text[i]<<" ("<<(unsigned int)bytes[i/2]<<")\n";
//	}
	
}

CompactSequence::~CompactSequence(){
	if(bytes != NULL){
		delete [] bytes;
		bytes = NULL;
	}
	n_bytes = 0;
}

unsigned int CompactSequence::getText(unsigned int position, unsigned int length, char *out){
	
	if(position >= text_size){
		return 0;
	}
	
	if( position + length >= text_size ){
		length = text_size - position;
	}
	
	char *salida = out;
//	char valor;
	
	//Si la posicion es impar, tomo un char primero
	if( (position & 0x1) == 1 ){
		switch( (*salida) >> 4 ){
			case 1:
				*out = 'A';
				break;
			case 2:
				*out = 'C';
				break;
			case 3:
				*out = 'G';
				break;
			case 4:
				*out = 'T';
				break;
			case 5:
				*out = 'N';
				break;
			default: 
				*out = 0;
				break;
		}
		++salida;
		++out;
		++position;
		--length;
	}
	
	//Pares de chars
//	for(unsigned int i = 0; i < length; ++i){
//		
//		switch( (*salida) ){
//			case 1:
//				*out = 'A';
//				break;
//			case 2:
//				*out = 'C';
//				break;
//			case 3:
//				*out = 'G';
//				break;
//			case 4:
//				*out = 'T';
//				break;
//			case 5:
//				*out = 'N';
//				break;
//			default: 
//				*out = 0;
//				break;
//		}
//	}
	
	//Si queda un ultimo char, lo tomo aqui
	
	
	return (salida - out);
}

char CompactSequence::getChar(unsigned int position){
	if(position >= text_size){
		return 0;
	}
	
	char c = 0;
	char valor = bytes[ (position >> 1) ];
	if( (position & 0x01) == 0){
		valor &= 0xf;
	}//if... par
	else{
		valor >>= 4;
	}//else... impar
	switch( valor ){
		case 1:
			c = 'A';
			break;
		case 2:
			c = 'C';
			break;
		case 3:
			c = 'G';
			break;
		case 4:
			c = 'T';
			break;
		case 5:
			c = 'N';
			break;
		default: 
			c = 0;
			break;
	}
	
	return c;
}

	
void CompactSequence::save(fstream *writer){
	if(writer == NULL || !(writer->good()) ){
		cerr<<"CompactSequence::save - Escritor dañado\n";
		return;
	}
	writer->write((char*)(&text_size), sizeof(int));
	writer->write((char*)(&n_bytes), sizeof(int));
	writer->write(bytes, n_bytes);
}

void CompactSequence::load(fstream *reader){
	if(reader == NULL || !(reader->good()) ){
		cerr<<"CompactSequence::load - Lector dañado\n";
		return;
	}
	if(bytes != NULL){
		delete [] bytes;
		bytes = NULL;
	}
	reader->read((char*)(&text_size), sizeof(int));
	reader->read((char*)(&n_bytes), sizeof(int));
	cout<<"CompactSequence::load - Cargando "<<n_bytes<<" bytes (texto de "<<text_size<<" chars)\n";
	bytes = new char[n_bytes];
	reader->read(bytes, n_bytes);
}









