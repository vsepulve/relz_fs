#ifndef _FILES_TREE_H
#define _FILES_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

//Para directorios y stat archivos
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
//unistd es para pathconf
#include <unistd.h>
//stddef es para offsetof
#include <stddef.h>

#include "DecoderBlocks.h"
#include "DecoderBlocksRelz.h"

using namespace std;

//Estas estructuras DEBEN SER THREAD-SAFE
//No deben usar funciones como strtok que no soportan concurrencia
//Idealmente, los metodos de lectura deberian ser const tambien

class FileNode {

private: 
	//bool para directorio, pero es posible que se agreguen todas las propiedas como bits de un entero o similar
	bool directory;
	unsigned long long size;
	
public: 
	//Valores para la estructura de arbol
	FileNode *first_child;
	FileNode *brother;
	
	//Valores del nodo (nombre y metadatos)
	string name;
	
	//Lectores del archivo real
	//Por ahora dejo solo un fstream, luego agregare un descompresor y un flag para marcar el correcto
	//Por ahora es de solo lectura, despues veremos como agregar la escritura
	fstream *lector;
	
	DecoderBlocks *decoder;
	
	//Variables estatica para compartir un puntero al texto de referencia
	static char *reference_text;
	

	FileNode();

	FileNode(const string &_name, bool _directory = false, unsigned long long _size = 0, DecoderBlocks *_decoder = NULL);
	
	virtual ~FileNode();
	
	//Retorna el ultimo hijo (para agregar y cosas similares)
	//Para esto itera por todos los hijos, podria guardar un puntero al ultimo
	FileNode *lastChild() const;
	
	//Retorna el hijo con el texto buscado (o NULL si no lo encuentra)
	//Esta version es lineal, y compara el texto de cada hijo
	//Si se agregaran alfabeticamente, podria hacerse una busqueda binaria
	FileNode *find(const string &_name) const;
	
	//Retorna el nodo recien creado
	//Siempre agrega el nuevo hijo al final
	//Esto podria modificarse para que los agregara en posicion alfabetica
	//Al agregar un hijo, convierto al nodo actual en directorio (ojo con esto)
	FileNode *add(const string &_name, bool _directory = false, unsigned long long _size = 0, DecoderBlocks *_decoder = NULL);
	
	//Revisa los hijos y borra (alguno) con el nombre pedido
	void removeChild(const string &_name);
	
	//LLamada recursiva
	void loadDirectory(const char *path);
	
	void print(unsigned int level = 0, bool recursive = true) const;
	
	bool isDirectory() const;
	
	unsigned long long getSize() const;
	
	void setSize(unsigned long long _size);
	
	//Notar que este metodo recibe el path al archivo REAL (no virtual)
	void updateNode(const char *real_path);
	
};

class FilesTree {

private: 
	
public: 

	//El arbol controla que root NUNCA tenga hermano, solo hijos
	//De este modo, cada nodo solo se preocupa de sus hijos, no de su hermano
	FileNode *root;
	
	//Ruta base del arbol (solo si fue cargado desde un directorio real)
	char *base_path;
	
	FilesTree();
	
	virtual ~FilesTree();
	
	void add(const char *path);
	
	void remove(const char *path);
	
	//Retorna el node asociado al path, o NULL si no lo encuentra
	FileNode *find(const char *path) const;
	
	//Esta version BORRA el contenido actual reemplazandolo por el contenido de un directorio
	//Notar que en el resultado, la raiz ("/") sera equivalente a path
	//guarda path en base_path
	void loadDirectory(const char *path);
	
	void print() const;
	
};




#endif //_FILES_TREE_H
