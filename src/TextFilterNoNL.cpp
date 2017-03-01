#include "TextFilterNoNL.h"

TextFilterNoNL::TextFilterNoNL(){
}

TextFilterNoNL::~TextFilterNoNL(){
}
	
// Retorna true para los chars validos para este TextFilterNoNL (No filtrados)
// Retoran false para todo char invalido (que sera filtrado)
bool TextFilterNoNL::validChar(char c){
	if( c == 0 || c == '\n' ){
		return false;
	}
	return true;
}

// Almacena el alfabeto valido completo en alphabet (si es NULL lo omite)
// Retorna el total de chars validos (incluso si alphabet == NULL)
unsigned int TextFilterNoNL::getAlphabet(vector<char> *alphabet){
	if(alphabet != NULL){
		for(unsigned int i = 1; i <= 0xff; ++i){
			if( (char)i != '\n' ){
				alphabet->push_back( (char)i );
			}
		}
	}
	//Notar que se agrega desde 1 hasta 0xff incluyendolo
	return 0xff;
}










