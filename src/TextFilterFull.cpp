#include "TextFilterFull.h"

TextFilterFull::TextFilterFull(){
}

TextFilterFull::~TextFilterFull(){
}
	
// Retorna true para los chars validos para este TextFilterFull (No filtrados)
// Retoran false para todo char invalido (que sera filtrado)
bool TextFilterFull::validChar(char c){
	if( c == 0 ){
		return false;
	}
	return true;
}

// Almacena el alfabeto valido completo en alphabet (si es NULL lo omite)
// Retorna el total de chars validos (incluso si alphabet == NULL)
unsigned int TextFilterFull::getAlphabet(vector<char> *alphabet){
	if(alphabet != NULL){
		for(unsigned int i = 1; i <= 0xff; ++i){
			alphabet->push_back( (char)i );
		}
	}
	//Notar que se agrega desde 1 hasta 0xff incluyendolo
	return 0xff;
}










