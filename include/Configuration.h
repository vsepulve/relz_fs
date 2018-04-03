#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <map>

using namespace std;

class Configuration{
private:
	
public:

	// Configuracion por defecto (sobreescrita con la lectura exitosa de un config)
	// ruta del directorio real
	char *base_path;
	// ruta de la referencia
	char *reference_file;
	// block_size para la compression
	unsigned int compress_block_size;
	// numero maximo de threads a ser usados para comprimir
	unsigned int compress_max_threads;
	// largo de bloque para descomprimir (se pide/borra esa ram por cada descompresion completa)
	unsigned int decompress_line_size;
	// Tamaño preferido para operaciones IO (por ejemplo, write)
	// Para usarlo apropiadamente, agrego "-o big_writes" al demonio
	unsigned int io_block_size;
	// Fin Configuracion

	Configuration();
	Configuration(string &filename);
	~Configuration();
	
	void loadConfiguration(string &filename);
	
};


#endif //_CONFIGURATION_H
