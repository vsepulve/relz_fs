#ifndef _TEXT_FILTER_H
#define _TEXT_FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>

#include "NanoTimer.h"

using namespace std;

class TextFilter{

protected: 
	
public: 
	
	TextFilter();
	
	virtual ~TextFilter();
	
	// Retorna true para los chars validos para este TextFilter (No filtrados)
	// Retoran false para todo char invalido (que sera filtrado)
	// El char '\0' (int 0) deberia ser invalido siempre (a menos que se resuelva de modo particular)
	virtual bool validChar(char c);
	
	// Almacena el alfabeto valido completo en alphabet (si es NULL lo omite)
	// Retorna el total de chars validos (incluso si alphabet == NULL)
	virtual unsigned int getAlphabet(vector<char> *alphabet = NULL);
	
	// Realiza la lectura y FILTRADO del texto y lo retorna como un c-string
	// El llamador debe encargarse de liberar la memoria del texto retornado
	// Escribe el largo real del texto en text_length (para ahorrar el strlen)
	// Almacena los runs (pares ini, fin) de lowcase solo si lowcase_runs != NULL (de otro modo, lo omite)
	// Este metodo utiliza validChar, por lo que no es necesario redefinirlo en la mayoria de los casos
	virtual char *readText(const char *in_file, unsigned long long &text_length, vector< pair<unsigned long long, unsigned long long> > *lowcase_runs = NULL);
	
	// Similar al anterior, pero diseñado especificamente para leer la referencia
	// Este metodo SOLO extrae los caracteres 'A', 'C', 'G' y 'T' (pasa las minusculas a mayusculas)
	virtual unsigned int readReference(const char *in_file, char *text);
	
	// Como el anterior pero deja pasar todo en A-Z, 0-9
	virtual unsigned int readReferenceFull(const char *in_file, char *text);
	
	// Revisa el texto eliminando los '\n' y guardando el largo de cada linea en nl_pos (si es != NULL)
	// Esta pensado para limpiar un texto YA FILTRADO por readText
	// Retorna el nuevo largo del string (igual a text_length - NL_eliminados)
	virtual unsigned long long filterNewLines(char *text, unsigned long long text_length, vector<unsigned long long> *nl_pos = NULL);
	
};







#endif //_TEXT_FILTER_H





