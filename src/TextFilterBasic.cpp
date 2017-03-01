#include "TextFilterBasic.h"

TextFilterBasic::TextFilterBasic(){
}

TextFilterBasic::~TextFilterBasic(){
}
	
// Retorna true para los chars validos para este TextFilterBasic (No filtrados)
// Retoran false para todo char invalido (que sera filtrado)
bool TextFilterBasic::validChar(char c){
	if(c == 'A' || c == 'C' || c == 'G' || c == 'N' || c == 'T'){
		return true;
	}
	return false;
}

// Almacena el alfabeto valido completo en alphabet (si es NULL lo omite)
// Retorna el total de chars validos (incluso si alphabet == NULL)
unsigned int TextFilterBasic::getAlphabet(vector<char> *alphabet){
	if(alphabet != NULL){
		alphabet->push_back('A');
		alphabet->push_back('C');
		alphabet->push_back('G');
		alphabet->push_back('N');
		alphabet->push_back('T');
	}
	return 5;
}










