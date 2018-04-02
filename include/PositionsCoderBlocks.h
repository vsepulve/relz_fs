#ifndef _POSITIONS_CODER_BLOCKS_H
#define _POSITIONS_CODER_BLOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "BitsUtils.h"

using namespace std;

//Por ahora esta clase incluye TODOS los algoritmos de escritura y lectura de posiciones.
//Notar que seria mejor tener una jerarquia de clases para los diferentes algoritmos.
//Uso este esquema exclusivamente por simpleza.

class PositionsCoderBlocks{

private: 
	BitsUtils utils;
	const char *nombre_archivo;
	unsigned int buff_size;
	unsigned int *buff;
	unsigned int byte_ini;
	
	void deleteBuffer();
	void prepareBuffer(unsigned int new_size);
	
public: 
	
	//Constructor de Lectura
	//Esperara un open para abrir un archivo y esperar lecturas
	PositionsCoderBlocks();
	
	//Constructor de Escritura
	//Crea el archivo (con trunc) para comenzar a escribir
	//Luego de las escrituras debe cerrarse con close
	PositionsCoderBlocks(const char *_nombre_archivo);

	virtual ~PositionsCoderBlocks();
	
	//Como el anterior, pero escribe en un archivo dado
	//Retorna el numero de bytes escritos
	unsigned int encodeBlockMaxBits(unsigned int *arr_pos, unsigned int n_factores, unsigned char max_bits, fstream *escritor);
	
	//Metodo de Escritura
	//Retorna el numero de bytes escritos
	unsigned int encodeBlockVarByte(unsigned int *arr_pos, unsigned int n_factores, fstream *escritor);
	
	//Metodo de Lectura
	void open(const char *_nombre_archivo, unsigned int _byte_ini = 0);
	
	//Metodo de Lectura
	//Lee el archivo (ya abierto) desde byte_start
	//Comienza leyendo max_bits y luego carga n_factores posiciones en arr_pos
	//Asume que arr_pos tiene al menos (n_factores + 1) ints de espacio
	void decodeBlockMaxBits(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_pos);
	
	//Metodo de Lectura
	//Lee el archivo (ya abierto) desde byte_start
	//Carga n_factores posiciones en arr_pos usando read_varbyte
	//Asume que arr_pos tiene al menos (n_factores + 1) ints de espacio
	void decodeBlockVarByte(unsigned int byte_start, unsigned int n_bytes, unsigned int n_factores, unsigned int *arr_pos);
	
};







#endif //_POSITIONS_CODER_H





