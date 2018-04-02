#ifndef _MUTATIONS_FILE_H
#define _MUTATIONS_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <map>

using namespace std;

// Notar que no almacena un identificador interno, asume que sera ordenado en un mapa o tabla externa
// Solo permito read, NO permitire write aqui (el llamador debe encargarse de negar eso)

class MutationsFile{
	
protected: 
	
	// Texto base
	unsigned int text_size;
	char *base_text;
	
	// Estructura de mutaciones
	unsigned int n_mut;
	// <pos, new_char>, version con pos uint (podria ser ull)
	pair<unsigned int, char> *arr_mut;
	
public:
	
	// Constructor vacio para agregar mutaciones o cargar archivo despues
	MutationsFile();
	
	// El puntero a base_text es compartido
	// Asume que el archivo esta en el formato correcto y ordenado
	// Reliza un par de verificaciones para asegurarlo (tamaño y orden de posiciones)
	// También verifico que el tamaño del texto base sea correcto
	MutationsFile(const char *file, char *_base_text, unsigned int _text_size);
	
	virtual ~MutationsFile();
	
	bool load(const char *file, char *_base_text, unsigned int _text_size);
	
	// Notar que este read siempre es exitoso (aunque puede parecer un archivo vacio)
	// Tambien notar que solo escribe size bytes en el buff, SIN AGREGAR un 0 final (para c-string)
	// Retorna el numero de bytes escritos en buff
	unsigned int read(char *buf, unsigned int size, unsigned long long offset);
	
	// Retorna el tamaño del texto base
	unsigned int size();
	
};







#endif //_MUTATIONS_FILE_H

