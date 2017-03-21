#ifndef _BLOCK_HEADERS_H
#define _BLOCK_HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>

#include "BytesReader.h"
#include "BitsUtils.h"
#include "Metadata.h"

using namespace std;

class BlockHeaders{

public:
	class Header{
	public:
		Header(){
		}
		virtual ~Header(){}
		
		// Retorna el tamaño en bytes de este header
		virtual unsigned int size(){
			return 0;
		}
		
		// Escribe este header directamente en el fstream
		// Debe escribir exactamente size() bytes
		virtual void save(fstream *file){}
		
		// Carga este header desde el fstream 
		// El llamador se encarga de prepara un Header del tipo correcto para los datos
		virtual void load(fstream *file){}
	};
	
protected: 
	
	// Variables globales
	unsigned int block_size;
	unsigned long long text_size;
	unsigned int data_pos;
	
	// Byte para iniciar la escritura
	// 0 inicialmente, o el primer header desacumulado en unprepare
	unsigned int bytes_total_initial;
	// Bloque inicial de la des-acumulacion
	unsigned int unprepared_block;
	
	//Para que addBlock sea valido (y polimorfico), DEBE mantenerse una coleccion de punteros
	//Los punteros tienen que ser guardados para mantener la memoria de cada header correctamente
	//Dejo comentada la solucion directa de un unico vector a Header* (la clase base)
	//Esto es para evitar el cast por cada acceso (y quizas solo usar cast en el add)
	//vector<Header*> headers;
	
	Metadata *metadata;
	
public: 
	
	BlockHeaders();
	
	// Notar que BlockHeaders toma posesion de _lowcase_runs y lo borrara en su destructor
	BlockHeaders(unsigned long long _text_size, unsigned int _block_size, Metadata *_metadata);
	
	virtual ~BlockHeaders();
	
	// Agrega los datos de un header particular a este objeto
	// Verifica que el tipo del header sea correcto
	virtual void addBlock(Header *header);
	
	// Carga un los datos de un header del fstream
	// Para ello crea un nuevo header del tipo correcto, lo carga con load
	// Luego agrega los datos de ese header a este objeto
	virtual void loadBlock(fstream *reader, unsigned int bytes);
	
	// Similar al anterior, pero reemplaza el bloque de pos si existe (si no, lo agrega)
	virtual void reloadBlock(fstream *reader, unsigned int bytes, unsigned int pos);
	
	// Guarda los datos de este BlockHeaders en un fstream
	// Esto incluye todos los datos propios incluyendo metadatos
	// Sin embargo NO almacena el tipo de headers (eso deberia hacerlo el factory)
	virtual unsigned int save(fstream *writer);
	
	// Carga todos los datos guardados por un save
	virtual void load(fstream *reader);
	
	// Igual al anterior, pero desde un BytesReader
	virtual void load(BytesReader *reader);
	
	// Retorna el tamaño de bloque
	virtual unsigned int getBlockSize();
	
	// Retorna el tamaño del texto (visto por un lector)
	virtual unsigned long long getTextSize();
	
	// Retorna la posicion de inicio absoluta de los datos
	virtual unsigned int getDataPosition();
	
	// Retorna la posicion absoluta de inicio de un bloque en el archivo terminado
	// Esto se usa en el write, para saber los bytes que deben preservarse
	// Notar que el inicio de un bloque se usa tambien para el final del anterior
	virtual unsigned int getBlockPosition(unsigned int block);
	
	// Retorna el numero (logico) de bloques
	virtual unsigned int getNumBlocks();
	
	// Prepara headers recien cargados para un save (incluyendo calculo de data_pos)
	// Acumula o ajusta las posiciones y agrega el bloque ficticio final (asume que dicho bloque no existe aun)
	virtual void prepare();
	
	// Deshace la preparacion desde un cierpo block_ini incluyendo la eliminacion del bloque ficticio
	// Des-acumula los valores desde ese bloque en adelante
	// ...guardando la posicion de ese bloque y su byte inicial para el proximo prepare
	virtual void unprepare(unsigned int block_ini = 0);
	
	// Retorna metadata (real, no una copia) de este objeto
	virtual Metadata *getMetadata();
	
	// Reemplaza el metadata de este objeto
	// No verifica ni destruye el anterior metadata
	// El llamador de este metodo debe encargarse de eliminar un potencial metadata anterior con getMetadata()
	virtual void setMetadata(Metadata *_metadata);
	
	//LLamadas directas a los metodos de metadata
	virtual void adjustCase(char *buff, unsigned long long ini, unsigned int length);
	virtual unsigned int countNewLines(unsigned long long pos);
	virtual void adjustNewLines(char *buff, unsigned long long ini, unsigned int length, unsigned int nl_izq, unsigned int nl_med, char *copy_buff = NULL);
	virtual void filterNewText(const char *in_buff, unsigned int length, unsigned long long pos_ini, char *out_buff, unsigned int &adjusted_length, unsigned long long &adjusted_pos_ini);
	
	// Este metodo especial es para el write (ajusta el text_size interno solo si es necesario)
	virtual void increaseTextSize(unsigned long long _text_size){
		if( _text_size > text_size ){
			text_size = _text_size;
		}
	}
	
};







#endif //_BLOCK_HEADERS_H





