#ifndef _LOCAL_BUFFER_H
#define _LOCAL_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <list>
#include <mutex>

using namespace std;

class LocalBuffer{

private: 
	// mutex global para operaciones estaticas
	static mutex global_mutex;
	// maximo de bloques reservados permitidos (const por ahora)
	static const unsigned int max_blocks = 10;
	// estructura para almacenar bloques estaticamente
	// vector o lista? creo que lista
	static list< pair< unsigned int, char* > > blocks;
	static unsigned int used_blocks;
	static pair<unsigned int, char*> arr_blocks[max_blocks];
	
	// memoria para uso directo
	char *mem;
	// largo minimo de la memoria
	unsigned int mem_size;
	// largo real de la memoria (real_size >= size)
	unsigned int real_size;
	
public: 
	
	LocalBuffer(unsigned int _size = 128);
	
	virtual ~LocalBuffer();
	
	// Metodo para obtener directamente la memoria interna
	// Notar que esto NO permite sobreescribir el puntero interno
	char *memory();
	
	// Largo (minimo) de la memoria interna
	unsigned int size();
	
	// Metodo estatico para limpiar la memoria interna
	// Notar que aun no tengo claro el uso de este metodo o si es necesario
	static void clearBlocks();
	
};







#endif //_LOCAL_BUFFER_H





